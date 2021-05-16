#include "SATScheduleGenerator.hpp"
#include "ScheduleCommon.hpp"

#include "absl/strings/numbers.h"
#include "absl/strings/str_split.h"
#include "ortools/base/commandlineflags.h"
#include "ortools/base/filelineiter.h"
#include "ortools/base/logging.h"
#include "ortools/sat/cp_model.h"
#include "ortools/sat/model.h"

#include <map>
#include <vector>
#include <utility>


using operations_research::sat::BoolVar;
using operations_research::sat::CpModelBuilder;
using operations_research::sat::LinearExpr;
using operations_research::sat::Model;
using operations_research::sat::CpSolverResponse;

using operations_research::sat::SolutionBooleanValue;
using operations_research::sat::NewSatParameters;
using operations_research::sat::NewFeasibleSolutionObserver;
using operations_research::sat::Solve;


using LessonsMtxItem = std::pair<LessonsMatrixItemAddress, BoolVar>;

struct LessonsMtxItemComp
{
    bool operator()(const LessonsMtxItem& lhs, const LessonsMtxItem& rhs) const
    {
        return lhs.first < rhs.first;
    }

    bool operator()(const LessonsMtxItem& lhs, const LessonsMatrixItemAddress& rhs) const
    {
        return lhs.first < rhs;
    }

    bool operator()(const LessonsMatrixItemAddress& lhs, const LessonsMtxItem& rhs) const
    {
        return lhs < rhs.first;
    }
};

class LessonsMatrix
{
public:
    void Add(const LessonsMatrixItemAddress& address, const BoolVar& boolVar)
    {
        auto it = std::lower_bound(elems_.begin(), elems_.end(), address, LessonsMtxItemComp());
        if(it == elems_.end() || it->first != address)
            elems_.emplace(it, address, boolVar);
        else
            it->second = boolVar;
    }

    const std::vector<LessonsMtxItem>& Elems() const { return elems_; }

    BoolVar* Find(const LessonsMatrixItemAddress& address)
    {
        auto it = std::lower_bound(elems_.begin(), elems_.end(), address, LessonsMtxItemComp());
        if(it == elems_.end() || it->first != address)
            return nullptr;

        return &it->second;
    }

    const BoolVar* Find(const LessonsMatrixItemAddress& address) const
    {
        return const_cast<LessonsMatrix*>(this)->Find(address);
    }

private:
    std::vector<LessonsMtxItem> elems_;
};


static void FillLessonsMatrix(CpModelBuilder& cp_model, LessonsMatrix& mtx, const ScheduleData& data)
{
    for (std::size_t g = 0; g < data.CountGroups(); ++g)
    {
        for (std::size_t p = 0; p < data.CountProfessors(); ++p)
        {
            for (std::size_t l = 0; l < MAX_LESSONS_COUNT; ++l)
            {
                const std::size_t day = LessonToScheduleDay(l);
                for (std::size_t c = 0; c < data.CountClassrooms(); ++c)
                {
                    for (std::size_t s = 0; s < data.CountSubjects(); ++s)
                    {
                        if (WeekDayRequestedForSubject(data, s, day) &&
                            ClassroomRequestedForSubject(data, s, c) &&
                            !data.LessonIsOccupied(LessonAddress(g, l)))
                        {
                            mtx.Add(LessonsMatrixItemAddress{g, p, l, c, s}, cp_model.NewBoolVar());
                        }
                    }
                }
            }
        }
    }
}

static void AddOneSubjectPerTimeCondition(CpModelBuilder& cp_model,
                                          const LessonsMatrix& lessons,
                                          const ScheduleData& data,
                                          std::vector<BoolVar>& buffer)
{
    for (std::size_t g = 0; g < data.CountGroups(); ++g)
    {
        for (std::size_t p = 0; p < data.CountProfessors(); ++p)
        {
            for (std::size_t l = 0; l < MAX_LESSONS_COUNT; ++l)
            {
                buffer.clear();
                for (std::size_t s = 0; s < data.CountSubjects(); ++s)
                {
                    for (std::size_t c = 0; c < data.CountClassrooms(); ++c)
                    {
                        const auto lesson = lessons.Find(LessonsMatrixItemAddress{g, p, l, c, s});
                        if (lesson)
                            buffer.emplace_back(*lesson);
                    }
                }

                cp_model.AddLessOrEqual(LinearExpr::BooleanSum(buffer), 1);
            }
        }
    }
}

static void AddSubjectsHoursCondition(CpModelBuilder& cp_model,
                                      const LessonsMatrix& lessons,
                                      const ScheduleData& data,
                                      std::vector<BoolVar>& buffer)
{
    // в сумме для одной группы за весь период должно быть ровно стролько пар, сколько выделено на каждый предмет
    for (std::size_t s = 0; s < data.CountSubjects(); ++s)
    {
        for (std::size_t g = 0; g < data.CountGroups(); ++g)
        {
            for (std::size_t p = 0; p < data.CountProfessors(); ++p)
            {
                buffer.clear();
                for (std::size_t l = 0; l < MAX_LESSONS_COUNT; ++l)
                {
                    for (std::size_t c = 0; c < data.CountClassrooms(); ++c)
                    {
                        const auto lesson = lessons.Find(LessonsMatrixItemAddress{g, p, l, c, s});
                        if (lesson)
                            buffer.emplace_back(*lesson);
                    }
                }

                cp_model.AddEquality(LinearExpr::BooleanSum(buffer), CalculateHours(data, p, g, s));
            }
        }
    }
}

static void AddMinimizeLatePairsCondition(CpModelBuilder& cp_model,
                                          const LessonsMatrix& lessons,
                                          const ScheduleData& data,
                                          std::vector<BoolVar>& buffer)
{
    // располагаем пары в начале дня
    buffer.clear();
    std::vector<std::int64_t> pairsCoefficients;
    for(auto&& item : lessons.Elems())
    {
        // [day, group, professor, lesson, classrooms, subject]
        const auto[g, p, l, c, s] = item.first;

        // чем позднее пара - тем выше коэффициент
        std::int64_t coeff = l;

        // +1 если пара в субботу
        coeff += (ScheduleDayNumberToWeekDay(LessonToScheduleDay(l)) == WeekDay::Saturday);

        buffer.emplace_back(item.second);
        pairsCoefficients.emplace_back(coeff);
    }

    cp_model.Minimize(LinearExpr::BooleanScalProd(buffer, pairsCoefficients));
}

static void AddMinimizeComplexity(CpModelBuilder& cp_model,
                                  const LessonsMatrix& lessons,
                                  const ScheduleData& data,
                                  std::vector<BoolVar>& buffer)
{
    for (std::size_t g = 0; g < data.CountGroups(); ++g)
    {
        for (std::size_t d = 0; d < SCHEDULE_DAYS_COUNT; ++d)
        {
            buffer.clear();
            std::vector<std::int64_t> sumComplexity;
            for (std::size_t s = 0; s < data.CountSubjects(); ++s)
            {
                const auto complexity = data.SubjectRequests().at(s).Complexity();
                for (std::size_t l = 0; l < MAX_LESSONS_PER_DAY; ++l)
                {
                    for (std::size_t c = 0; c < data.CountClassrooms(); ++c)
                    {
                        for (std::size_t p = 0; p < data.CountProfessors(); ++p)
                        {
                            const auto lesson = lessons.Find(LessonsMatrixItemAddress{g, p, d * MAX_LESSONS_PER_DAY + l, c, s});
                            if(lesson == nullptr)
                                continue;

                            buffer.emplace_back(*lesson);
                            sumComplexity.emplace_back(complexity);
                        }
                    }
                }
            }

            if(!buffer.empty())
                cp_model.Minimize(LinearExpr::BooleanScalProd(buffer, sumComplexity));
        }
    }
}

static void AddStreamsCondition(CpModelBuilder& cp_model,
                                const LessonsMatrix& lessons,
                                const ScheduleData& data)
{
    for (std::size_t s = 0; s < data.CountSubjects(); ++s)
    {
        for (std::size_t p = 0; p < data.CountProfessors(); ++p)
        {
            for (std::size_t l = 0; l < MAX_LESSONS_COUNT; ++l)
            {
                for (std::size_t c = 0; c < data.CountClassrooms(); ++c)
                {
                    std::optional<BoolVar> prevVar;
                    for (std::size_t g = 0; g < data.CountGroups(); ++g)
                    {
                        if (!data.SubjectRequests().at(s).RequestedGroup(g))
                            continue;

                        const auto lesson = lessons.Find(LessonsMatrixItemAddress{g, p, l, c, s});
                        if(lesson == nullptr)
                            continue;

                        if(prevVar)
                        {
                            cp_model.AddEquality(*prevVar, *lesson);
                        }
                        else
                        {
                            prevVar.emplace(*lesson);
                        }
                    }
                }
            }
        }
    }
}

static void AddConditions(CpModelBuilder& cp_model,
                          const LessonsMatrix& lessons,
                          const ScheduleData& data)
{
    std::vector<BoolVar> buffer;
    AddOneSubjectPerTimeCondition(cp_model, lessons, data, buffer);
    AddStreamsCondition(cp_model, lessons, data);
    AddSubjectsHoursCondition(cp_model, lessons, data, buffer);
    AddMinimizeComplexity(cp_model, lessons, data, buffer);
    AddMinimizeLatePairsCondition(cp_model, lessons, data, buffer);
}

static ScheduleResult MakeScheduleFromSolverResponse(const CpSolverResponse& response,
                                                       const LessonsMatrix& lessons,
                                                       const ScheduleData& data)
{
    ScheduleResult resultSchedule;
    for (std::size_t g = 0; g < data.CountGroups(); ++g)
    {
        for (std::size_t l = 0; l < MAX_LESSONS_COUNT; ++l)
        {
            for(std::size_t p = 0; p < data.CountProfessors(); ++p)
            {
                for (std::size_t c = 0; c < data.CountClassrooms(); ++c)
                {
                    for (std::size_t s = 0; s < data.CountSubjects(); ++s)
                    {
                        const auto lesson = lessons.Find(LessonsMatrixItemAddress{g, p, l, c, s});
                        if (lesson && SolutionBooleanValue(response, *lesson))
                        {
                            resultSchedule.insert(ScheduleItem(LessonAddress(g, l), s, p, c));
                        }
                    }
                }
            }
        }
    }

    return resultSchedule;
}


ScheduleResult SATScheduleGenerator::Generate(const ScheduleData& data)
{
    CpModelBuilder cp_model;
    LessonsMatrix lessons;

    FillLessonsMatrix(cp_model, lessons, data);
    AddConditions(cp_model, lessons, data);

    const CpSolverResponse response = Solve(cp_model.Build());
    LOG(INFO) << CpSolverResponseStats(response);
    if(!response.IsInitialized())
        return ScheduleResult({});

    return MakeScheduleFromSolverResponse(response, lessons, data);
}

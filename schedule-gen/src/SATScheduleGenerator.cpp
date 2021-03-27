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


ScheduleResult SATScheduleGenerator::Genetate(const ScheduleData& data) {
    using operations_research::sat::BoolVar;
    using operations_research::sat::CpModelBuilder;
    using operations_research::sat::LinearExpr;
    using operations_research::sat::Model;
    using operations_research::sat::CpSolverResponse;

    using operations_research::sat::SolutionBooleanValue;
    using operations_research::sat::NewSatParameters;
    using operations_research::sat::NewFeasibleSolutionObserver;
    using operations_research::sat::Solve;


    CpModelBuilder cp_model;

    // [day, group, professor, lesson, classrooms, subject]
    using mtx_index = std::tuple<std::size_t, std::size_t, std::size_t, std::size_t, std::size_t, std::size_t>;
    std::map<mtx_index, BoolVar> lessons;

    // создание переменных
    for (std::size_t d = 0; d < SCHEDULE_DAYS_COUNT; ++d) {
        for (std::size_t g = 0; g < data.CountGroups(); ++g) {
            for (std::size_t p = 0; p < data.CountProfessors(); ++p) {
                for (std::size_t l = 0; l < data.MaxCountLessonsPerDay(); ++l) {
                    for (std::size_t c = 0; c < data.CountClassrooms(); ++c) {
                        for (std::size_t s = 0; s < data.CountSubjects(); ++s) {
                            if (WeekDayRequestedForSubject(data, s, d) && ClassroomRequestedForSubject(data, s, c))
                                lessons[{d, g, p, l, c, s}] = cp_model.NewBoolVar();
                        }
                    }
                }
            }
        }
    }

    // в одно время может быть только один предмет
    {
        std::vector<BoolVar> sumSubjectsInOneMoment;
        for (std::size_t g = 0; g < data.CountGroups(); ++g) {
            for (std::size_t p = 0; p < data.CountProfessors(); ++p) {
                for (std::size_t d = 0; d < SCHEDULE_DAYS_COUNT; ++d) {
                    for (std::size_t l = 0; l < data.MaxCountLessonsPerDay(); ++l) {
                        sumSubjectsInOneMoment.clear();
                        for (std::size_t s = 0; s < data.CountSubjects(); ++s) {
                            for (std::size_t c = 0; c < data.CountClassrooms(); ++c) {
                                const auto it = lessons.find({d, g, p, l, c, s});
                                if (it != lessons.end())
                                    sumSubjectsInOneMoment.emplace_back(it->second);
                            }
                        }

                        cp_model.AddLessOrEqual(LinearExpr::BooleanSum(sumSubjectsInOneMoment), 1);
                    }
                }
            }
        }
    }

    {
        // в сумме для одной группы за весь период должно быть ровно стролько пар, сколько выделено на каждый предмет
        std::vector<BoolVar> sumSubjectHours;
        for (std::size_t s = 0; s < data.CountSubjects(); ++s)
        {
            for (std::size_t g = 0; g < data.CountGroups(); ++g)
            {
                for (std::size_t p = 0; p < data.CountProfessors(); ++p)
                {
                    sumSubjectHours.clear();

                    for (std::size_t d = 0; d < SCHEDULE_DAYS_COUNT; ++d)
                    {
                        for (std::size_t l = 0; l < data.MaxCountLessonsPerDay(); ++l)
                        {
                            for (std::size_t c = 0; c < data.CountClassrooms(); ++c)
                            {
                                const auto it = lessons.find({d, g, p, l, c, s});
                                if (it != lessons.end())
                                    sumSubjectHours.emplace_back(it->second);
                            }
                        }
                    }

                    cp_model.AddEquality(LinearExpr::BooleanSum(sumSubjectHours), CalculateHours(data, p, g, s));
                }
            }
        }
    }

    {
        // располагаем пары в начале дня, стараясь не превышать data.RequestedCountLessonsPerDay()
        std::vector<BoolVar> pairsPerDay;
        std::vector<std::int64_t> pairsCoefficients;
        for (std::size_t d = 0; d < SCHEDULE_DAYS_COUNT; ++d) {
            for (std::size_t g = 0; g < data.CountGroups(); ++g) {
                for (std::size_t p = 0; p < data.CountProfessors(); ++p) {
                    for (std::size_t l = 0; l < data.MaxCountLessonsPerDay(); ++l) {
                        for (std::size_t c = 0; c < data.CountClassrooms(); ++c) {
                            for (std::size_t s = 0; s < data.CountSubjects(); ++s) {
                                const auto it = lessons.find({d, g, p, l, c, s});
                                if (it == lessons.end())
                                    continue;

                                // чем позднее пара - тем выше коэффициент
                                std::int64_t coeff = l;

                                // +1 если пара превышает желаемое количество пар в день
                                coeff += (l >= data.RequestedCountLessonsPerDay());

                                // +1 если пара в субботу
                                coeff += (ScheduleDayNumberToWeekDay(d) == WeekDay::Saturday);

                                pairsPerDay.emplace_back(it->second);
                                pairsCoefficients.emplace_back(coeff);
                            }
                        }
                    }
                }
            }
        }

        cp_model.Minimize(LinearExpr::BooleanScalProd(pairsPerDay, pairsCoefficients));
    }

    const CpSolverResponse response = Solve(cp_model.Build());
    LOG(INFO) << CpSolverResponseStats(response);
    if(!response.IsInitialized())
        return ScheduleResult({});

    std::vector<ScheduleResult::Group> resultScheduleGroups;
    for (std::size_t g = 0; g < data.CountGroups(); ++g)
    {
        ScheduleResult::Group resultScheduleGroup;
        for (std::size_t d = 0; d < SCHEDULE_DAYS_COUNT; ++d)
        {
            ScheduleResult::Day resultScheduleDay;
            for (std::size_t l = 0; l < data.MaxCountLessonsPerDay(); ++l)
            {
                ScheduleResult::Lesson resultScheduleLesson = std::nullopt;
                for(std::size_t p = 0; p < data.CountProfessors(); ++p)
                {
                    for (std::size_t c = 0; c < data.CountClassrooms(); ++c)
                    {
                        for (std::size_t s = 0; s < data.CountSubjects(); ++s)
                        {
                            const auto it = lessons.find({d, g, p, l, c, s});
                            if (it != lessons.end() && SolutionBooleanValue(response, it->second))
                            {
                                resultScheduleLesson.emplace(s, p, c);
                            }
                        }
                    }
                }
                resultScheduleDay.emplace_back(std::move(resultScheduleLesson));
            }
            resultScheduleGroup.emplace_back(std::move(resultScheduleDay));
        }
        resultScheduleGroups.emplace_back(std::move(resultScheduleGroup));
    }

    return ScheduleResult(std::move(resultScheduleGroups));
}

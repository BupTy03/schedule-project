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


ScheduleResult SATScheduleGenerator::Genetate(const ScheduleData& data)
{
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

    // [day, group, professor, lesson, classroom, subject]
    using mtx_index = std::tuple<std::size_t, std::size_t, std::size_t, std::size_t, std::size_t, std::size_t>;
    std::map<mtx_index, BoolVar> lessons;

    // создание переменных
    for (std::size_t d = 0; d < SCHEDULE_DAYS_COUNT; ++d)
    {
        for (std::size_t g = 0; g < data.CountGroups(); ++g)
        {
            for(std::size_t p = 0; p < data.CountProfessors(); ++p)
            {
                for (std::size_t l = 0; l < data.MaxCountLessonsPerDay(); ++l)
                {
                    for(std::size_t c = 0; c < data.CountClassrooms(); ++c)
                    {
                        for (std::size_t s = 0; s < data.CountSubjects(); ++s)
                            lessons[{d, g, p, l, c, s}] = cp_model.NewBoolVar();
                    }
                }
            }
        }
    }

    // 1. каждому конкретному предмету соответствует свой конкретный преподаватель
    // 2. в сумме для одной группы для одного преподавателя для одного предмета
    // за весь период должно быть ровно стролько часов, сколько выделено на каждый предмет
    for (std::size_t g = 0; g < data.CountGroups(); ++g)
    {
        for (std::size_t p = 0; p < data.CountProfessors(); ++p)
        {
            for (std::size_t s = 0; s < data.CountSubjects(); ++s)
            {
                std::vector<BoolVar> sumDays;
                sumDays.reserve(SCHEDULE_DAYS_COUNT * data.MaxCountLessonsPerDay() * data.CountClassrooms());
                for (std::size_t d = 0; d < SCHEDULE_DAYS_COUNT; ++d)
                {
                    for (std::size_t l = 0; l < data.MaxCountLessonsPerDay(); ++l)
                    {
                        for (std::size_t c = 0; c < data.CountClassrooms(); ++c)
                            sumDays.emplace_back(lessons[{d, g, p, l, c, s}]);
                    }
                }

                cp_model.AddEquality(LinearExpr::BooleanSum(sumDays), CalculateHours(data, p, g, s));
            }
        }
    }

    // 3. в одно время в одной и той же аудитории может быть только один предмет
    for (std::size_t d = 0; d < SCHEDULE_DAYS_COUNT; ++d)
    {
        for (std::size_t g = 0; g < data.CountGroups(); ++g)
        {
            for (std::size_t p = 0; p < data.CountProfessors(); ++p)
            {
                for (std::size_t l = 0; l < data.MaxCountLessonsPerDay(); ++l)
                {
                    for(std::size_t c = 0; c < data.CountClassrooms(); ++c)
                    {
                        std::vector<BoolVar> sumSubjects;
                        sumSubjects.reserve(data.CountSubjects());
                        for (std::size_t s = 0; s < data.CountSubjects(); ++s)
                        {
                            sumSubjects.emplace_back(lessons[{d, g, p, l, c, s}]);
                        }

                        cp_model.AddLessOrEqual(LinearExpr::BooleanSum(sumSubjects), 1);
                    }
                }
            }
        }
    }

    // 4. учитываем дни, в которые преподаватели не желают работать
    for (std::size_t s = 0; s < data.CountSubjects(); ++s)
    {
        for (std::size_t d = 0; d < SCHEDULE_DAYS_COUNT; ++d)
        {
            if(WeekDayRequestedForSubject(data, s, ScheduleDayNumberToWeekDay(d)))
                continue;

            std::vector<BoolVar> sumForDay;
            sumForDay.reserve(data.CountProfessors() * data.CountGroups() *
                                    data.MaxCountLessonsPerDay() * data.CountClassrooms());

            const auto& subjRequest = data.SubjectRequests().at(s);
            const std::size_t p = subjRequest.Professor();
            for (std::size_t g = 0; g < data.CountGroups(); ++g)
            {
                if(!subjRequest.RequestedGroup(g))
                    continue;

                for (std::size_t l = 0; l < data.MaxCountLessonsPerDay(); ++l)
                {
                    for (std::size_t c = 0; c < data.CountClassrooms(); ++c)
                    {
                        sumForDay.emplace_back(lessons[{d, g, p, l, c, s}]);
                    }
                }
            }

            cp_model.AddEquality(LinearExpr::BooleanSum(sumForDay), 0);
        }
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
                            if (SolutionBooleanValue(response, lessons[{d, g, p, l, c, s}]))
                            {
                                resultScheduleLesson.emplace(s, p, c);
                                break;
                            }
                        }

                        if (resultScheduleLesson)
                            break;
                    }

                    if (resultScheduleLesson)
                        break;
                }
                resultScheduleDay.emplace_back(std::move(resultScheduleLesson));
            }
            resultScheduleGroup.emplace_back(std::move(resultScheduleDay));
        }
        resultScheduleGroups.emplace_back(std::move(resultScheduleGroup));
    }

    return ScheduleResult(std::move(resultScheduleGroups));
}

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

    // [day, group, professor, lesson, classrooms, subject]
    using mtx_index = std::tuple<std::size_t, std::size_t, std::size_t, std::size_t, std::size_t, std::size_t>;
    std::map<mtx_index, BoolVar> lessons;

    // создание переменных
    for (std::size_t d = 0; d < SCHEDULE_DAYS_COUNT; ++d)
        for (std::size_t g = 0; g < data.CountGroups(); ++g)
            for (std::size_t p = 0; p < data.CountProfessors(); ++p)
                for (std::size_t l = 0; l < data.MaxCountLessonsPerDay(); ++l)
                    for (std::size_t c = 0; c < data.CountClassrooms(); ++c)
                        for (std::size_t s = 0; s < data.CountSubjects(); ++s)
                            lessons[{d, g, p, l, c, s}] = cp_model.NewBoolVar();

    // в одно время может быть только один предмет
    for (std::size_t g = 0; g < data.CountGroups(); ++g)
    {
        for(std::size_t p = 0; p < data.CountProfessors(); ++p)
        {
            for (std::size_t d = 0; d < SCHEDULE_DAYS_COUNT; ++d)
            {
                for (std::size_t l = 0; l < data.MaxCountLessonsPerDay(); ++l)
                {
                    std::vector<BoolVar> sumSubjects;
                    for (std::size_t s = 0; s < data.CountSubjects(); ++s)
                        for (std::size_t c = 0; c < data.CountClassrooms(); ++c)
                            sumSubjects.emplace_back(lessons[{d, g, p, l, c, s}]);

                    cp_model.AddLessOrEqual(LinearExpr::BooleanSum(sumSubjects), 1);
                }
            }
        }
    }

    // в сумме для одной группы за весь период должно быть ровно стролько пар, сколько выделено на каждый предмет
    for (std::size_t s = 0; s < data.CountSubjects(); ++s)
    {
        for (std::size_t g = 0; g < data.CountGroups(); ++g)
        {
            for (std::size_t p = 0; p < data.CountProfessors(); ++p)
            {
                std::vector<BoolVar> sumDays;
                for (std::size_t d = 0; d < SCHEDULE_DAYS_COUNT; ++d)
                    for (std::size_t l = 0; l < data.MaxCountLessonsPerDay(); ++l)
                        for (std::size_t c = 0; c < data.CountClassrooms(); ++c)
                            sumDays.emplace_back(lessons[{d, g, p, l, c, s}]);

                cp_model.AddEquality(LinearExpr::BooleanSum(sumDays), CalculateHours(data, p, g, s));
            }
        }
    }

    // учитываем дни в которые преподы не желают работать
    std::vector<BoolVar> notWorkingDaysSum;
    for (std::size_t s = 0; s < data.CountSubjects(); ++s)
    {
        for (std::size_t d = 0; d < SCHEDULE_DAYS_COUNT; ++d)
        {
            if(WeekDayRequestedForSubject(data, s, d))
                continue;

            for (std::size_t g = 0; g < data.CountGroups(); ++g)
            {
                for (std::size_t p = 0; p < data.CountProfessors(); ++p)
                {
                    for (std::size_t l = 0; l < data.MaxCountLessonsPerDay(); ++l)
                    {
                        for (std::size_t c = 0; c < data.CountClassrooms(); ++c)
                        {
                            notWorkingDaysSum.emplace_back(lessons[{d, g, p, l, c, s}]);
                        }
                    }
                }
            }
        }
    }
    cp_model.AddEquality(LinearExpr::BooleanSum(notWorkingDaysSum), 0);

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

#include "GAScheduleGenerator.hpp"

#include "ga/GAGenome.h"
#include "ga/GASimpleGA.h"
#include "ga/GADemeGA.h"
#include "ga/GASStateGA.h"
#include "ga/GAIncGA.h"
#include "ga/GA1DBinStrGenome.h"


static float Objective(GAGenome&);


struct GAScheduleGeneratorContext
{
public:
    explicit GAScheduleGeneratorContext(const ScheduleData* pScheduleData)
        : pScheduleData_(pScheduleData)
    {
        assert(ScheduleDataPtr != nullptr);
        for (std::size_t d = 0; d < SCHEDULE_DAYS_COUNT; ++d)
        {
            for (std::size_t g = 0; g < pScheduleData_->CountGroups(); ++g)
            {
                for (std::size_t p = 0; p < pScheduleData_->CountProfessors(); ++p)
                {
                    for (std::size_t l = 0; l < pScheduleData_->MaxCountLessonsPerDay(); ++l)
                    {
                        for (std::size_t c = 0; c < pScheduleData_->CountClassrooms(); ++c)
                        {
                            for (std::size_t s = 0; s < pScheduleData_->CountSubjects(); ++s)
                            {
                                if (WeekDayRequestedForSubject(*pScheduleData_, s, d) &&
                                    ClassroomRequestedForSubject(*pScheduleData_, s, c) &&
                                    !pScheduleData_->LessonIsOccupied(LessonAddress(g, d, l)))
                                {
                                    lessonItems_.emplace_back(d, g, p, l, c, s);
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    [[nodiscard]] const ScheduleData& Data() const { return *pScheduleData_; }
    [[nodiscard]] const std::vector<LessonsMatrixItemAddress>& Lessons() const
    {
        return lessonItems_;
    }

private:
    const ScheduleData* pScheduleData_;
    std::vector<LessonsMatrixItemAddress> lessonItems_;
};


ScheduleResult GAScheduleGenerator::Generate(const ScheduleData& data)
{
    constexpr int populationSize = 10;
    constexpr int generationsCount = 10;
    constexpr float pmut = 0.9;
    constexpr float pcross = 0.9;

    GAScheduleGeneratorContext context(&data);
    GA1DBinaryStringGenome genome(context.Lessons().size(), Objective, &context);
    //GASimpleGA ga(genome);
    GASteadyStateGA ga(genome);
    ga.populationSize(populationSize);
    ga.nGenerations(generationsCount);
//    ga.pMutation(pmut);
//    ga.pCrossover(pcross);

    GARankSelector selector;
    ga.selector(selector);

    GASigmaTruncationScaling sigmaTruncation;
    ga.scaling(sigmaTruncation);

    ga.initialize();
    ga.evolve();
    genome = ga.statistics().bestIndividual();

    std::vector<ScheduleItem> resultSchedule;
    for(std::size_t i = 0; i < genome.size(); ++i)
    {
        if(genome.gene(i))
        {
            const auto& item = context.Lessons().at(i);
            resultSchedule.emplace_back(LessonAddress(item.Group, item.Day, item.Lesson), item.Subject, item.Professor, item.Classroom);

            std::cout << "[g:" << item.Group << ",d:" << item.Day << ",l:" << item.Lesson << "] {s:" << item.Subject << ",p:" << item.Professor << ",c:" << item.Classroom << "}\n";
        }
    }

    std::cout << "resultSchedule.size(): " << resultSchedule.size() << '\n';
    std::cout << "Best score: " << genome.score() << std::endl;
    return ScheduleResult(std::move(resultSchedule));
}


static float Objective(GAGenome& gn)
{
    auto&& genome = dynamic_cast<GA1DBinaryStringGenome&>(gn);
    auto pContext = reinterpret_cast<GAScheduleGeneratorContext*>(genome.userData());
    const auto& data = pContext->Data();
    const auto& lessons = pContext->Lessons();

    float score = 0.f;
    for (std::size_t g = 0; g < data.CountGroups(); ++g)
    {
        for (std::size_t p = 0; p < data.CountProfessors(); ++p)
        {
            for (std::size_t d = 0; d < SCHEDULE_DAYS_COUNT; ++d)
            {
                for (std::size_t l = 0; l < data.MaxCountLessonsPerDay(); ++l)
                {
                    float sum = 0.f;
                    for (std::size_t s = 0; s < data.CountSubjects(); ++s)
                    {
                        for (std::size_t c = 0; c < data.CountClassrooms(); ++c)
                        {
                            const LessonsMatrixItemAddress idx(d, g, p, l, c, s);
                            const auto it = std::lower_bound(lessons.begin(), lessons.end(), idx);
                            if(it != lessons.end() && *it == idx && genome.gene(std::distance(lessons.begin(), it)))
                                sum += 1.f;
                        }
                    }

                    score -= std::fabs(sum - 1.f);
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
                float sum = 0.f;
                for (std::size_t d = 0; d < SCHEDULE_DAYS_COUNT; ++d)
                {
                    for (std::size_t l = 0; l < data.MaxCountLessonsPerDay(); ++l)
                    {
                        for (std::size_t c = 0; c < data.CountClassrooms(); ++c)
                        {
                            const LessonsMatrixItemAddress idx(d, g, p, l, c, s);
                            const auto it = std::lower_bound(lessons.begin(), lessons.end(), idx);
                            if(it != lessons.end() && *it == idx && genome.gene(std::distance(lessons.begin(), it)))
                                sum += 1.f;
                        }
                    }
                }

                score -= std::fabs(sum - CalculateHours(data, p, g, s));
            }
        }
    }

    //std::cout << "Score: " << score << '\n';
    return score;
}

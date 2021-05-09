#include "GAScheduleGenerator.hpp"


void Print(const ScheduleIndividual& individ, const std::vector<SubjectRequest>& requests)
{
    const auto& lessons = individ.Lessons();
    const auto& classrooms = individ.Classrooms();

    for(std::size_t l = 0; l < MAX_LESSONS_COUNT; ++l)
    {
        std::cout << "Lesson " << l << ": ";

        auto it = std::find(lessons.begin(), lessons.end(), l);
        if(it == lessons.end())
        {
            std::cout << '-';
        }
        else
        {
            while(it != lessons.end())
            {
                const std::size_t r = std::distance(lessons.begin(), it);
                const auto& request = requests.at(r);
                std::cout << "[s:" << r << ", p:" << request.Professor() << ", c:" << classrooms.at(r) << "]";

                it = std::find(std::next(it), lessons.end(), l);
            }
        }

        std::cout << '\n';
    }
}

ScheduleResult GAScheduleGenerator::Generate(const ScheduleData& data)
{
    std::vector<SubjectRequest> requests;
    std::vector<std::size_t> subjectNumbers;

    const auto& subjectRequests = data.SubjectRequests();
    for(std::size_t subject = 0; subject < subjectRequests.size(); ++subject)
    {
        auto&& r = subjectRequests.at(subject);
        requests.insert(requests.end(), r.HoursPerWeek(), r);
        subjectNumbers.insert(subjectNumbers.end(), r.HoursPerWeek(), subject);
    }

    ScheduleGA algo(std::vector<ScheduleIndividual>(1000, ScheduleIndividual(requests)));
    const auto stat = algo.IterationsCount(1000)
            .SelectionCount(200)
            .CrossoverCount(25)
            .MutationChance(45)
            .Start(requests);

    const auto& bestIndividual = algo.Best();
    Print(bestIndividual, requests);
    std::cout << "Best: " << bestIndividual.Evaluate(requests) << '\n';
    std::cout << "Time: " << std::chrono::duration_cast<std::chrono::seconds>(stat.Time).count() << "s.\n";
    std::cout << "Iterations: " << stat.Iterations << '\n';
    std::cout.flush();

    const auto& lessons = bestIndividual.Lessons();
    const auto& classrooms = bestIndividual.Classrooms();

    std::vector<ScheduleItem> scheduleItems;
    for(std::size_t l = 0; l < MAX_LESSONS_COUNT; ++l)
    {
        auto it = std::find(lessons.begin(), lessons.end(), l);
        while(it != lessons.end())
        {
            const std::size_t r = std::distance(lessons.begin(), it);
            const auto& request = requests.at(r);
            for(std::size_t g : request.Groups())
            {
                scheduleItems.emplace_back(LessonAddress(g,
                                                         l / MAX_LESSONS_PER_DAY,
                                                         l % MAX_LESSONS_PER_DAY),
                                           subjectNumbers.at(r),
                                           request.Professor(),
                                           classrooms.at(r));
            }

            it = std::find(std::next(it), lessons.end(), l);
        }
    }

    return ScheduleResult(std::move(scheduleItems));
}

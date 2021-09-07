#include "ScheduleIndividual.h"
#include "ScheduleData.h"


ScheduleIndividual::ScheduleIndividual(std::random_device& randomDevice,
                                       const ScheduleData* pData)
    : pData_(pData)
    , evaluatedValue_(NOT_EVALUATED)
    , chromosomes_(InitializeChromosomes(*pData))
    , randomGenerator_(randomDevice())
{
    assert(pData != nullptr);
}

void ScheduleIndividual::swap(ScheduleIndividual& other) noexcept
{
    std::swap(evaluatedValue_, other.evaluatedValue_);
    std::swap(chromosomes_, other.chromosomes_);
}

ScheduleIndividual::ScheduleIndividual(const ScheduleIndividual& other)
    : pData_(other.pData_)
    , evaluatedValue_(other.evaluatedValue_)
    , chromosomes_(other.chromosomes_)
    , randomGenerator_(other.randomGenerator_)
{
}

ScheduleIndividual& ScheduleIndividual::operator=(const ScheduleIndividual& other)
{
    ScheduleIndividual tmp(other);
    tmp.swap(*this);
    return *this;
}

ScheduleIndividual::ScheduleIndividual(ScheduleIndividual&& other) noexcept
    : pData_(other.pData_)
    , evaluatedValue_(other.evaluatedValue_)
    , chromosomes_(std::move(other.chromosomes_))
    , randomGenerator_(other.randomGenerator_)
{
}

ScheduleIndividual& ScheduleIndividual::operator=(ScheduleIndividual&& other) noexcept
{
    other.swap(*this);
    return *this;
}

std::size_t ScheduleIndividual::MutationProbability() const
{
    std::uniform_int_distribution<std::size_t> mutateDistrib(0, 100);
    return mutateDistrib(randomGenerator_);
}

void ScheduleIndividual::Mutate()
{
    std::uniform_int_distribution<std::size_t> requestsDistrib(0, pData_->SubjectRequests().size() - 1);
    const std::size_t requestIndex = requestsDistrib(randomGenerator_);

    std::uniform_int_distribution<std::size_t> headsOrTails(0, 1);
    if(headsOrTails(randomGenerator_))
        ChangeClassroom(requestIndex);
    else
        ChangeLesson(requestIndex);
}

std::size_t ScheduleIndividual::Evaluate() const
{
    if(evaluatedValue_ != NOT_EVALUATED)
        return evaluatedValue_;

    evaluatedValue_ = ::Evaluate(chromosomes_, *pData_);
    return evaluatedValue_;
}

void ScheduleIndividual::Crossover(ScheduleIndividual& other)
{
    std::uniform_int_distribution<std::size_t> requestsDist(0, pData_->SubjectRequests().size() - 1);
    const auto requestIndex = requestsDist(randomGenerator_);
    if(ReadyToCrossover(chromosomes_, other.chromosomes_, *pData_, requestIndex))
    {
        evaluatedValue_ = NOT_EVALUATED;
        other.evaluatedValue_ = NOT_EVALUATED;
        ::Crossover(chromosomes_, other.chromosomes_, requestIndex);
    }
}


void ScheduleIndividual::ChangeClassroom(std::size_t requestIndex)
{
    const auto& request = pData_->SubjectRequests().at(requestIndex);
    const auto& classrooms = request.Classrooms();

    if(classrooms.empty())
        return;

    std::uniform_int_distribution<std::size_t> classroomDistrib(0, classrooms.size() - 1);
    auto scheduleClassroom = classrooms.at(classroomDistrib(randomGenerator_));

    std::size_t chooseClassroomTry = 0;
    while(chooseClassroomTry < classrooms.size() &&
          chromosomes_.ClassroomsIntersects(chromosomes_.Lesson(requestIndex), scheduleClassroom))
    {
        scheduleClassroom = classrooms.at(classroomDistrib(randomGenerator_));
        ++chooseClassroomTry;
    }

    if(chooseClassroomTry < classrooms.size())
    {
        chromosomes_.Classroom(requestIndex) = scheduleClassroom;
        evaluatedValue_ = NOT_EVALUATED;
    }
}

void ScheduleIndividual::ChangeLesson(std::size_t requestIndex)
{
    const auto& request = pData_->SubjectRequests().at(requestIndex);
    if(pData_->SubjectRequestHasLockedLesson(request))
        return;

    std::uniform_int_distribution<std::size_t> lessonsDistrib(0, MAX_LESSONS_COUNT - 1);
    std::size_t scheduleLesson = lessonsDistrib(randomGenerator_);

    std::size_t chooseLessonTry = 0;
    if(request.IsEveningClass())
    {
        while(chooseLessonTry < MAX_LESSONS_COUNT &&
          (!request.RequestedWeekDay(scheduleLesson / MAX_LESSONS_PER_DAY) ||
           !SuitableForEveningClasses(scheduleLesson) ||
           chromosomes_.GroupsOrProfessorsOrClassroomsIntersects(*pData_, requestIndex, scheduleLesson)))
        {
            scheduleLesson = lessonsDistrib(randomGenerator_);
            ++chooseLessonTry;
        }
    }
    else
    {
        while(chooseLessonTry < MAX_LESSONS_COUNT &&
          (!request.RequestedWeekDay(scheduleLesson / MAX_LESSONS_PER_DAY) ||
           IsLateScheduleLessonInSaturday(scheduleLesson) ||
           chromosomes_.GroupsOrProfessorsOrClassroomsIntersects(*pData_, requestIndex, scheduleLesson)))
        {
            scheduleLesson = lessonsDistrib(randomGenerator_);
            ++chooseLessonTry;
        }
    }

    if(chooseLessonTry < MAX_LESSONS_COUNT)
    {
        chromosomes_.Lesson(requestIndex) = scheduleLesson;
        evaluatedValue_ = NOT_EVALUATED;
    }
}

void swap(ScheduleIndividual& lhs, ScheduleIndividual& rhs) { lhs.swap(rhs); }

void Print(const ScheduleIndividual& individ,
           const ScheduleData& data)
{
    const auto& requests = data.SubjectRequests();
    const auto& lessons = individ.Chromosomes().Lessons();
    const auto& classrooms = individ.Chromosomes().Classrooms();

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
                std::cout << "[s:" << request.ID() <<
                          ", p:" << request.Professor() <<
                          ", c:(" << classrooms.at(r).Building << ", " << classrooms.at(r).Classroom << "), g: {";

                for(auto&& g : request.Groups())
                    std::cout << ' ' << g;

                std::cout << " }]";

                it = std::find(std::next(it), lessons.end(), l);
            }
        }

        std::cout << '\n';
    }
}

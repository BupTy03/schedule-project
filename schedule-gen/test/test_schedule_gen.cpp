#include "ScheduleCommon.hpp"
#include "ScheduleData.hpp"
#include "ScheduleResult.hpp"

#include <catch2/catch.hpp>

#include <array>


static std::vector<ClassroomAddress> MakeClassroomsRange(std::size_t n)
{
    std::vector<ClassroomAddress> result;
    result.reserve(n);
    for(std::size_t i = 0; i < n; ++i)
        result.emplace_back(0, i);

    return result;
}

TEST_CASE("TestScheduleDayNumberToWeekDay", "[utils]")
{
    const std::array<WeekDay, 12> scheduleDays = {
            WeekDay::Monday,
            WeekDay::Tuesday,
            WeekDay::Wednesday,
            WeekDay::Thursday,
            WeekDay::Friday,
            WeekDay::Saturday,
            WeekDay::Monday,
            WeekDay::Tuesday,
            WeekDay::Wednesday,
            WeekDay::Thursday,
            WeekDay::Friday,
            WeekDay::Saturday
    };

    for(std::size_t d = 0; d < scheduleDays.size(); ++d)
        REQUIRE(ScheduleDayNumberToWeekDay(d) == scheduleDays.at(d));
}


TEST_CASE("Test.WeekDays.contains", "[WeekDays]")
{
    WeekDays days;
    REQUIRE(days.contains(WeekDay::Monday));
    REQUIRE(days.contains(WeekDay::Tuesday));
    REQUIRE(days.contains(WeekDay::Wednesday));
    REQUIRE(days.contains(WeekDay::Thursday));
    REQUIRE(days.contains(WeekDay::Friday));
    REQUIRE(days.contains(WeekDay::Saturday));
}

TEST_CASE("Test.WeekDays.erase", "[WeekDays]")
{
    auto checkRemove = [](const WeekDays& weekDays, WeekDay removed){
        for(int i = 0; i < 6; ++i)
        {
            const auto wd = static_cast<WeekDay>(i);
            if(removed == wd)
            {
                REQUIRE_FALSE(weekDays.contains(wd));
            }
            else
            {
                REQUIRE(weekDays.contains(wd));
            }
        }
    };

    for(int i = 0; i < 6; ++i)
    {
        const auto wd = static_cast<WeekDay>(i);
        WeekDays days;
        days.erase(wd);
        checkRemove(days, wd);
    }

    WeekDays days;
    for(int i = 0; i < 6; ++i)
        days.erase(static_cast<WeekDay>(i));

    REQUIRE(days.empty());
}

TEST_CASE("Test.WeekDays.insert", "[WeekDays]")
{
    for(int i = 0; i < 6; ++i)
    {
        WeekDays weekDays = WeekDays::emptyWeek();
        weekDays.insert(static_cast<WeekDay>(i));
        for(int j = 0; j < 6; ++j)
        {
            if(i == j)
                REQUIRE(weekDays.contains(static_cast<WeekDay>(j)));
            else
                REQUIRE_FALSE(weekDays.contains(static_cast<WeekDay>(j)));
        }
    }
}

TEST_CASE("Test.WeekDays.WeekDaysIterator", "[WeekDays]")
{
    for(int i = 0; i < 6; ++i)
    {
        const auto removed = static_cast<WeekDay>(i);

        WeekDays weekDays;
        weekDays.erase(removed);

        auto it = weekDays.begin();
        for(int j = 0; j < 6; ++j)
        {
            if(i == j)
                REQUIRE_FALSE(*it);
            else
                REQUIRE(*it);

            ++it;
        }
        REQUIRE(it == weekDays.end());
    }
}

TEST_CASE("Test.SortedSet.construct", "[SortedSet]")
{
    std::array<int, 20> arr = {4, 6, 2, 9, 1, 3, 5, 7, 8, 0,
                               4, 2, 6, 1, 3, 0, 7, 5, 2, 6};
    std::array<int, 10> expected = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};

    SortedSet<int> sortedSet(arr.begin(), arr.end());
    REQUIRE(sortedSet.size() == expected.size());
    REQUIRE(std::equal(sortedSet.begin(), sortedSet.end(), expected.begin()));
}

TEST_CASE("Test.SortedSet.contains", "[SortedSet]")
{
    SortedSet<int> emptySet;
    for(auto e : {1, 2, 3, 6})
        REQUIRE_FALSE(emptySet.contains(e));

    std::array<int, 10> arr = {4, 6, 2, 9, 0, 6, 1, 0, 5, 1};
    SortedSet<int> sortedSet(arr.begin(), arr.end());
    for(std::size_t i = 0; i < 100; ++i)
    {
        if(std::find(arr.begin(), arr.end(), i) == arr.end())
            REQUIRE_FALSE(sortedSet.contains(i));
        else
            REQUIRE(sortedSet.contains(i));
    }
}

TEST_CASE("Test.SortedSet.insert", "[SortedSet]")
{
    const std::pair<int, std::vector<int>> testCases[] = {
            {0, {0}},
            {9, { 0, 9 }},
            {4, { 0, 4, 9 }},
            {-1, { -1, 0, 4, 9 }},
            {2, { -1, 0, 2, 4, 9 }},
            {7, {-1, 0, 2, 4, 7, 9}},
            {0, {-1, 0, 2, 4, 7, 9}},
            {4, {-1, 0, 2, 4, 7, 9}}
    };

    SortedSet<int> sortedSet;
    for(auto[toAdd, expected] : testCases)
    {
        sortedSet.insert(toAdd);
        REQUIRE(sortedSet.size() == expected.size());
        REQUIRE(std::equal(sortedSet.begin(), sortedSet.end(), expected.begin()));
    }
}

TEST_CASE("Test.SortedSet.erase", "[SortedSet]")
{
    const std::pair<int, std::vector<int>> testCases[] = {
            {4, {-1, 0, 2, 7, 9}},
            {0, {-1, 2, 7, 9}},
            {7, {-1, 2, 9}},
            {2, {-1, 9}},
            {-1, {9}},
            {4, {9}},
            {9, {}},
            {0, {}},
    };

    const auto& initialList = testCases[0].second;
    SortedSet<int> sortedSet(initialList.begin(), initialList.end());
    for(auto[toRemove, expected] : testCases)
    {
        sortedSet.erase(toRemove);
        REQUIRE(sortedSet.size() == expected.size());
        REQUIRE(std::equal(sortedSet.begin(), sortedSet.end(), expected.begin()));
    }
}

TEST_CASE("Test.SortedMap.construct", "[SortedMap]")
{
    SortedMap<std::string, int> words = {
            {"pen",      1},
            {"apple", 3},
            {"penapple", 0},
            {"applepen", 2}
    };

    REQUIRE(words.elems() == std::vector<std::pair<std::string, int>>{
        {"apple", 3},
        {"applepen", 2},
        {"pen",      1},
        {"penapple", 0}
    });
}

TEST_CASE("Test.SortedMap.operator[]", "[SortedMap]")
{
    SortedMap<std::string, int> words = {
            {"pen",      1},
            {"apple", 3},
            {"penapple", 0},
            {"applepen", 2}
    };

    // inserting elements
    words["pen"] = 2;
    words["apple"] = 0;
    words["penapple"] = 3;
    words["applepen"] = 1;

    REQUIRE(words.elems() == std::vector<std::pair<std::string, int>>{
            {"apple", 0},
            {"applepen", 1},
            {"pen",      2},
            {"penapple", 3}
    });

    // changing existing elements
    words["pen"] = 1;
    words["apple"] = 3;
    words["penapple"] = 0;
    words["applepen"] = 2;

    REQUIRE(words.elems() == std::vector<std::pair<std::string, int>>{
            {"apple", 3},
            {"applepen", 2},
            {"pen",      1},
            {"penapple", 0}
    });
}

TEST_CASE("Test.set_intersects", "[Algorithms]")
{
    REQUIRE(set_intersects(std::vector<int>({1, 2, 3, 4, 5}), std::vector<int>({3, 4})));
    REQUIRE(set_intersects(std::vector<int>({1, 2, 3, 4, 5}), std::vector<int>({1, 2, 3, 4, 5})));
    REQUIRE(set_intersects(std::vector<int>(), std::vector<int>()));
    REQUIRE(set_intersects(std::vector<int>({}), std::vector<int>({3, 4})));
    REQUIRE(set_intersects(std::vector<int>({1, 2, 3, 4, 5}), std::vector<int>({})));
    REQUIRE(set_intersects(std::vector<int>({1, 2, 3, 6, 7, 8, 9, 10}), std::vector<int>({1, 2, 4, 5, 6, 7, 9, 10})));

    REQUIRE_FALSE(set_intersects(std::vector<int>({1, 2, 3, 4, 5}), std::vector<int>({6, 7, 8})));
    REQUIRE_FALSE(set_intersects(std::vector<int>({1, 2, 5}), std::vector<int>({3, 4})));
}

TEST_CASE("Test.CalculatePadding", "[Utils]")
{
    REQUIRE(CalculatePadding(3, 2) == 1);
    REQUIRE(CalculatePadding(3, 4) == 1);
    REQUIRE(CalculatePadding(2, 4) == 2);
    REQUIRE(CalculatePadding(1, 8) == 7);

    REQUIRE(CalculatePadding(0, 0) == 0);
    REQUIRE(CalculatePadding(0, 8) == 0);
    REQUIRE(CalculatePadding(1, 0) == 0);
}

TEST_CASE("Test.FindOverlappedClassrooms", "[Validate]")
{
//    explicit SubjectRequest(std::size_t id,
//        std::size_t professor,
//        std::size_t complexity,
//        WeekDays days,
//        std::vector<std::size_t> groups,
//        std::vector<ClassroomAddress> classrooms);

    const std::vector<SubjectRequest> subjectRequests = {
        SubjectRequest(0, 1, 1, {}, {0}, MakeClassroomsRange(3)),
        SubjectRequest(1, 1, 1, {}, {3}, MakeClassroomsRange(3)),
        SubjectRequest(2, 1, 2, {}, {1}, MakeClassroomsRange(3)),
        SubjectRequest(3, 1, 3, {}, {2}, MakeClassroomsRange(3)),
        SubjectRequest(4, 1, 4, {}, {1}, MakeClassroomsRange(3))
    };

//    explicit ScheduleData(std::vector<std::size_t> groups,
//        std::vector<std::size_t> professors,
//        std::vector<ClassroomAddress> classrooms,
//        std::vector<SubjectRequest> subjectRequests,
//        std::vector<SubjectWithAddress> occupiedLessons);

    const ScheduleData scheduleData(MakeIndexesRange(6),
                                    MakeIndexesRange(5),
                                    MakeClassroomsRange(3),
                                    subjectRequests,
                                    {});
//    std::size_t Address;
//    std::size_t SubjectRequest;
//    std::size_t SubjectRequestID;
//    std::size_t Classroom;

    ScheduleResult scheduleResult;
    scheduleResult.insert(ScheduleItem(0, 0, 0, 0));
    scheduleResult.insert(ScheduleItem(0, 1, 1, 0));
    scheduleResult.insert(ScheduleItem(0, 2, 2, 1));
    scheduleResult.insert(ScheduleItem(0, 3, 3, 2));
    scheduleResult.insert(ScheduleItem(0, 4, 4, 1));

    const auto result = FindOverlappedClassrooms(scheduleData, scheduleResult);
    REQUIRE(result.size() == 2);
    REQUIRE(std::find_if(result.begin(), result.end(), [](auto&& oc){ return oc.Classroom == 0; }) != result.end());
    REQUIRE(std::find_if(result.begin(), result.end(), [](auto&& oc){ return oc.Classroom == 1; }) != result.end());
}

#if 0

TEST_CASE("Test.FindOverlappedProfessors", "[Validate]")
{
    const std::vector<SubjectRequest> subjectRequests = {
            SubjectRequest(0, 1, 0, {}, {0}, {0, 1, 2}),
            SubjectRequest(1, 1, 1, {}, {3}, {0, 1, 2}),
            SubjectRequest(2, 1, 2, {}, {1}, {0, 1, 2}),
            SubjectRequest(3, 1, 3, {}, {2}, {0, 1, 2}),
            SubjectRequest(4, 1, 4, {}, {1}, {0, 1, 2})
    };

    ScheduleData scheduleData(MakeIndexesRange(6),
                              MakeIndexesRange(5),
                              MakeIndexesRange(3),
                              subjectRequests,
                              {});

    ScheduleResult scheduleResult;
    scheduleResult.insert(ScheduleItem(LessonAddress(0, 0), 0, 0, 0));
    scheduleResult.insert(ScheduleItem(LessonAddress(3, 0), 1, 0, 0));
    scheduleResult.insert(ScheduleItem(LessonAddress(4, 0), 2, 1, 1));
    scheduleResult.insert(ScheduleItem(LessonAddress(2, 0), 3, 3, 2));
    scheduleResult.insert(ScheduleItem(LessonAddress(1, 0), 4, 1, 1));

    const auto result = FindOverlappedProfessors(scheduleData, scheduleResult);
    REQUIRE(result.size() == 2);
    REQUIRE(std::find_if(result.begin(), result.end(), [](auto&& oc){ return oc.Professor == 0; }) != result.end());
    REQUIRE(std::find_if(result.begin(), result.end(), [](auto&& oc){ return oc.Professor == 1; }) != result.end());
}

TEST_CASE("Test.FindViolatedSubjectRequests", "[Validate]")
{
    const std::vector<SubjectRequest> subjectRequests = {
            SubjectRequest(0, 1, 0, {WeekDay::Friday}, {0}, {0, 1, 2}),
            SubjectRequest(1, 1, 1, {WeekDay::Monday, WeekDay::Friday}, {3}, {0, 1, 2}),
            SubjectRequest(2, 1, 2, {WeekDay::Thursday, WeekDay::Friday}, {1}, {0, 1, 2}),
            SubjectRequest(3, 1, 3, {WeekDay::Saturday, WeekDay::Friday}, {2}, {0, 1, 2}),
            SubjectRequest(4, 1, 4, {WeekDay::Saturday, WeekDay::Wednesday}, {1}, {0, 1, 2})
    };

    const std::vector<SubjectWithAddress> fixedLessons = {
            SubjectWithAddress(0, LessonAddress(0, 0)),
            SubjectWithAddress(4, LessonAddress(1, 1 * MAX_LESSONS_PER_DAY + 4))
    };

    const ScheduleData scheduleData(MakeIndexesRange(6),
                                    MakeIndexesRange(5),
                                    MakeIndexesRange(3),
                                    subjectRequests,
                                    fixedLessons);

    ScheduleResult scheduleResult;
    scheduleResult.insert(ScheduleItem(LessonAddress(0, 0), 0, 0, 0));
    scheduleResult.insert(ScheduleItem(LessonAddress(3, 1), 1, 1, 1));
    scheduleResult.insert(ScheduleItem(LessonAddress(4, 3 * MAX_LESSONS_PER_DAY + 2), 2, 2, 2));
    scheduleResult.insert(ScheduleItem(LessonAddress(2, 5 * MAX_LESSONS_PER_DAY + 3), 3, 3, 0));
    scheduleResult.insert(ScheduleItem(LessonAddress(1, 1 * MAX_LESSONS_PER_DAY + 4), 4, 4, 2));

    const auto result = FindViolatedSubjectRequests(scheduleData, scheduleResult);
    REQUIRE(result.size() == 2);
    REQUIRE(std::find_if(result.begin(), result.end(), [](auto&& oc){ return oc.Lessons.contains(LessonAddress(0, 0)); }) != result.end());
    REQUIRE(std::find_if(result.begin(), result.end(), [](auto&& oc){ return oc.Lessons.contains(LessonAddress(1, 1 * MAX_LESSONS_PER_DAY + 4)); }) != result.end());
}

#endif

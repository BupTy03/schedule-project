#include "ScheduleUtils.h"
#include "ScheduleCommon.h"
#include "ScheduleData.h"
#include "ScheduleResult.h"
#include <catch2/catch.hpp>
#include <array>


TEST_CASE("SubjectRequest constructs correctly", "[subject_request]")
{
    SECTION("Sorting and removing duplicates from groups list while constructing")
    {
        // set_intersects algorithm REQUIRES sorted groups
        const SubjectRequest sut{0, 1, 1, {}, {3, 1, 2, 2, 5, 0, 10, 5, 1, 3, 3}, {}};
        REQUIRE(sut.Groups() == std::vector<std::size_t>{0, 1, 2, 3, 5, 10});
    }
}

TEST_CASE("Week day requested check performs correctly", "[subject_request]")
{
    const SubjectRequest sut{0, 1, 1, {WeekDay::Monday, WeekDay::Wednesday, WeekDay::Thursday},
                              {1, 2, 3}, {{0,0}}};

    // Monday numerator/denominator is requested
    REQUIRE(sut.RequestedWeekDay(0));
    REQUIRE(sut.RequestedWeekDay(6));

    // Wednesday numerator/denominator is requested
    REQUIRE(sut.RequestedWeekDay(2));
    REQUIRE(sut.RequestedWeekDay(8));

    // Thursday numerator/denominator is requested
    REQUIRE(sut.RequestedWeekDay(3));
    REQUIRE(sut.RequestedWeekDay(9));

    // Other days are not requested
    for(int wd : {1, 4, 5, 7, 10, 11})
        REQUIRE_FALSE(sut.RequestedWeekDay(wd));
}

TEST_CASE("Search by subject id performs correctly", "[schedule_data]")
{
    const std::vector subjectRequests{
        SubjectRequest{0, 1, 1, {}, {0}, {}},
        SubjectRequest{1, 2, 1, {}, {1}, {}},
        SubjectRequest{2, 3, 2, {}, {2}, {}},
        SubjectRequest{3, 4, 3, {}, {3}, {}},
        SubjectRequest{4, 5, 4, {}, {4}, {}}
    };
    const ScheduleData sut{subjectRequests,{}};

    SECTION("Searching subject request by id")
    {
        for(auto&& request : sut.SubjectRequests())
            REQUIRE(sut.SubjectRequestAtID(request.ID()) == request);

        REQUIRE_THROWS(sut.SubjectRequestAtID(5));
        REQUIRE_THROWS(sut.SubjectRequestAtID(101));
    }
    SECTION("Searching index in requests list by subject id works")
    {
        for(std::size_t i = 0; i < sut.SubjectRequests().size(); ++i)
        {
            auto&& request = sut.SubjectRequests().at(i);
            REQUIRE(sut.IndexOfSubjectRequestWithID(request.ID()) == i);
        }

        REQUIRE_THROWS(sut.IndexOfSubjectRequestWithID(5));
        REQUIRE_THROWS(sut.IndexOfSubjectRequestWithID(100));
    }
}

TEST_CASE("Check if subject request has locked lesson works", "[schedule_data]")
{
    const std::vector subjectRequests{
        SubjectRequest{0, 1, 1, {}, {0}, {}},
        SubjectRequest{1, 2, 1, {}, {1}, {}},
        SubjectRequest{2, 3, 2, {}, {2}, {}},
        SubjectRequest{3, 4, 3, {}, {3}, {}},
        SubjectRequest{4, 5, 4, {}, {4}, {}}
    };
    const std::vector lockedLessons {
        SubjectWithAddress{0, 0},
        SubjectWithAddress{3, 2},
        SubjectWithAddress{4, 1}
    };
    const ScheduleData sut{subjectRequests, lockedLessons};


    REQUIRE(sut.SubjectRequestHasLockedLesson(subjectRequests.at(0)));
    REQUIRE(sut.SubjectRequestHasLockedLesson(subjectRequests.at(3)));
    REQUIRE(sut.SubjectRequestHasLockedLesson(subjectRequests.at(4)));

    REQUIRE_FALSE(sut.SubjectRequestHasLockedLesson(subjectRequests.at(1)));
    REQUIRE_FALSE(sut.SubjectRequestHasLockedLesson(subjectRequests.at(2)));
}

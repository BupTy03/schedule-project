#include "GAScheduleGenerator.hpp"
#include <catch2/catch.hpp>


TEST_CASE("Check if groups or professors or classrooms intersects", "[ScheduleChromosomes]")
{
    const std::vector<ClassroomAddress> classrooms = {{0, 1}, {0, 2}, {0, 3}};
    const std::vector requests {
        // [id, professor, complexity, weekDays, groups, classrooms]
        SubjectRequest(0, 1, 1, {}, {0, 1, 2}, classrooms),
        SubjectRequest(1, 2, 1, {}, {1, 2, 3}, classrooms),
        SubjectRequest(2, 1, 1, {}, {4, 5, 6}, classrooms),
        SubjectRequest(3, 4, 1, {}, {7, 8, 9}, classrooms),
        SubjectRequest(4, 5, 1, {}, {10},      classrooms)
    };
    const ScheduleData data{requests, {}};
    const ScheduleChromosomes sut{{0, 1, 2, 3, 4},
                                  {{0,1}, {0,2}, {0,3}, {0,1}, {0,2}}};

    SECTION("Check if groups intersects")
    {
        REQUIRE(sut.GroupsOrProfessorsIntersects(data, 1, 0));
        REQUIRE(sut.GroupsOrProfessorsOrClassroomsIntersects(data, 1, 0));
    }
    SECTION("Check if professors intersects")
    {
        REQUIRE(sut.GroupsOrProfessorsIntersects(data, 2, 2));
        REQUIRE(sut.GroupsOrProfessorsOrClassroomsIntersects(data, 2, 2));
    }
    SECTION("Check if classrooms intersects")
    {
        REQUIRE(sut.ClassroomsIntersects(0, {0, 1}));
        REQUIRE(sut.GroupsOrProfessorsOrClassroomsIntersects(data, 3, 0));
    }
}

TEST_CASE("Check if chromosomes ready for crossover", "[ScheduleChromosomes]")
{
    const std::vector<ClassroomAddress> classrooms = {{0, 1}, {0, 2}, {0, 3}};
    const std::vector requests {
        // [id, professor, complexity, weekDays, groups, classrooms]
        SubjectRequest(0, 1, 1, {}, {0, 1, 2}, classrooms),
        SubjectRequest(1, 2, 1, {}, {1, 2, 3}, classrooms),
        SubjectRequest(2, 1, 1, {}, {4, 5, 6}, classrooms),
        SubjectRequest(3, 4, 1, {}, {7, 8, 9}, classrooms),
        SubjectRequest(4, 5, 1, {}, {10},      classrooms)
    };
    const ScheduleData data{requests, {}};

    ScheduleChromosomes sut1{{0, 1, 2, 3, 4}, {{0,3}, {0,2}, {0,1}, {0,3}, {0,2}}};
    ScheduleChromosomes sut2{{4, 3, 2, 1, 0}, {{0,1}, {0,2}, {0,3}, {0,2}, {0,2}}};

    REQUIRE(ReadyToCrossover(sut1, sut2, data, 0));
    REQUIRE_FALSE(ReadyToCrossover(sut1, sut2, data, 1));
    REQUIRE_FALSE(ReadyToCrossover(sut1, sut2, data, 2));
    REQUIRE_FALSE(ReadyToCrossover(sut1, sut2, data, 3));
    REQUIRE(ReadyToCrossover(sut1, sut2, data, 4));
}

TEST_CASE("Crossover works", "[ScheduleChromosomes]")
{
    const std::vector<ClassroomAddress> classrooms = {{0, 1}, {0, 2}, {0, 3}};
    const std::vector requests {
        // [id, professor, complexity, weekDays, groups, classrooms]
        SubjectRequest(0, 1, 1, {}, {0, 1, 2}, classrooms),
        SubjectRequest(1, 2, 1, {}, {1, 2, 3}, classrooms),
        SubjectRequest(2, 1, 1, {}, {4, 5, 6}, classrooms),
        SubjectRequest(3, 4, 1, {}, {7, 8, 9}, classrooms),
        SubjectRequest(4, 5, 1, {}, {10},      classrooms)
    };
    const ScheduleData data{requests, {}};

    ScheduleChromosomes first{{0, 1, 2, 3, 4}, {{0,3}, {0,2}, {0,1}, {0,3}, {0,2}}};
    ScheduleChromosomes second{{4, 3, 2, 1, 0}, {{0,1}, {0,2}, {0,3}, {0,1}, {0,2}}};

    Crossover(first, second, 0);
    REQUIRE(first.Lesson(0) == 4);
    REQUIRE(first.Classroom(0) == ClassroomAddress{0,1});

    REQUIRE(second.Lesson(0) == 0);
    REQUIRE(second.Classroom(0) == ClassroomAddress{0,3});
}

#pragma once
#include "ScheduleCommon.h"
#include "ScheduleUtils.h"

#include <unordered_map>
#include <unordered_set>
#include <vector>


class SubjectRequest
{
public:
    SubjectRequest() = default;
    explicit SubjectRequest(std::size_t id,
                            std::size_t professor,
                            std::size_t complexity,
                            std::vector<std::size_t> groups,
                            std::vector<std::size_t> lessons,
                            std::vector<ClassroomAddress> classrooms);

    std::size_t ID() const { return id_; }
    std::size_t Professor() const { return professor_; }
    std::size_t Complexity() const { return complexity_; }
    const std::vector<std::size_t>& Groups() const { return groups_; }
    const std::vector<ClassroomAddress>& Classrooms() const { return classrooms_; }
    const std::vector<std::size_t>& Lessons() const;

    friend bool operator==(const SubjectRequest& lhs, const SubjectRequest& rhs)
    {
        return lhs.id_ == rhs.id_ && lhs.professor_ == rhs.professor_
               && lhs.complexity_ == rhs.complexity_ && lhs.groups_ == rhs.groups_
               && lhs.lessons_ == rhs.lessons_ && lhs.classrooms_ == rhs.classrooms_;
    }

    friend bool operator!=(const SubjectRequest& lhs, const SubjectRequest& rhs)
    {
        return !(lhs == rhs);
    }

private:
    std::size_t id_ = 0;
    std::size_t professor_ = 0;
    std::size_t complexity_ = 0;
    std::vector<std::size_t> groups_;
    std::vector<std::size_t> lessons_;
    std::vector<ClassroomAddress> classrooms_;
};


struct SubjectRequestIDLess
{
    bool operator()(const SubjectRequest& lhs, const SubjectRequest& rhs) const
    {
        return lhs.ID() < rhs.ID();
    }
    bool operator()(const SubjectRequest& lhs, std::size_t rhsID) const { return lhs.ID() < rhsID; }
    bool operator()(std::size_t lhsID, const SubjectRequest& rhs) const { return lhsID < rhs.ID(); }
};

struct SubjectRequestIDEqual
{
    bool operator()(const SubjectRequest& lhs, const SubjectRequest& rhs) const
    {
        return lhs.ID() == rhs.ID();
    }
};


class SubjectsBlock
{
public:
    explicit SubjectsBlock(std::vector<std::size_t> requestIndexes,
                           std::vector<std::size_t> addresses)
        : requests_(std::move(requestIndexes))
        , lessons_(std::move(addresses))
    {
    }

    const std::vector<std::size_t>& Requests() const { return requests_; }
    const std::vector<std::size_t>& Addresses() const { return lessons_; }

private:
    std::vector<std::size_t> requests_;
    std::vector<std::size_t> lessons_;
};


class ScheduleData
{
public:
    ScheduleData() = default;
    explicit ScheduleData(std::vector<SubjectRequest> subjectRequests,
                          std::vector<SubjectsBlock> blocks = {});

    const std::vector<SubjectRequest>& SubjectRequests() const { return subjectRequests_; }
    const std::vector<SubjectsBlock>& Blocks() const { return blocks_; }

    const SubjectRequest& SubjectRequestAtID(std::size_t subjectRequestID) const;
    SubjectRequest& SubjectRequestAtID(std::size_t subjectRequestID);

    std::size_t IndexOfSubjectRequestWithID(std::size_t subjectRequestID) const;

    bool Intersects(std::size_t lhsSubjectRequest, std::size_t rhsSubjectRequest) const;

    const std::unordered_map<std::size_t, std::unordered_set<std::size_t>>& Professors() const
    {
        return professorRequests_;
    }
    const std::unordered_map<std::size_t, std::unordered_set<std::size_t>>& Groups() const
    {
        return groupRequests_;
    }

    bool IsInBlock(std::size_t subjectRequestIndex) const;
    const SubjectsBlock* FindBlockByRequestIndex(std::size_t subjectRequestIndex) const;

private:
    std::vector<SubjectRequest> subjectRequests_;
    BitIntersectionsMatrix intersectionsTable_;
    std::vector<SubjectsBlock> blocks_;
    std::unordered_map<std::size_t, std::size_t> requestsBlocks_;
    std::unordered_map<std::size_t, std::unordered_set<std::size_t>> professorRequests_;
    std::unordered_map<std::size_t, std::unordered_set<std::size_t>> groupRequests_;
};


SubjectsBlock ToSubjectsBlock(const std::vector<SubjectRequest>& requests,
                              const std::vector<std::size_t>& requestIDs);

std::vector<std::size_t> SelectBlockFirstLessons(const std::vector<SubjectRequest>& requests,
                                                 const std::vector<std::size_t>& block);

std::vector<SubjectsBlock> ToSubjectsBlocks(const std::vector<SubjectRequest>& requests,
                                            const std::vector<std::vector<std::size_t>>& blocksIds);

BitIntersectionsMatrix FillIntersectionsMatrix(const std::vector<SubjectRequest>& requests);

std::unordered_map<std::size_t, std::size_t> FillRequestsBlocksTable(const std::vector<SubjectsBlock>& blocks);

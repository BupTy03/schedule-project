#include "ScheduleData.h"

#include "ScheduleUtils.h"

#include <algorithm>
#include <string>


SubjectRequest::SubjectRequest(std::size_t id,
                               std::size_t professor,
                               std::size_t complexity,
                               std::vector<std::size_t> groups,
                               std::vector<std::size_t> lessons,
                               std::vector<ClassroomAddress> classrooms)
    : id_(id)
    , professor_(professor)
    , complexity_(complexity)
    , groups_(std::move(groups))
    , lessons_(std::move(lessons))
    , classrooms_(std::move(classrooms))
{
    assert(std::ranges::is_sorted(groups_));
    assert(std::ranges::is_sorted(lessons_));
    assert(std::ranges::is_sorted(classrooms_));
}

const std::vector<std::size_t>& SubjectRequest::Lessons() const
{
    return lessons_.empty() ? AllLessons() : lessons_;
}


ScheduleData::ScheduleData(std::vector<SubjectRequest> subjectRequests,
                           std::vector<SubjectsBlock> blocks)
    : subjectRequests_(std::move(subjectRequests))
    , intersectionsTable_(FillIntersectionsMatrix(subjectRequests_))
    , blocks_(std::move(blocks))
    , requestsBlocks_(FillRequestsBlocksTable(blocks_))
    , professorRequests_()
    , groupRequests_()
{
    assert(std::ranges::is_sorted(subjectRequests_, {}, &SubjectRequest::ID));

    for(std::size_t r = 0; r < subjectRequests_.size(); ++r)
    {
        const auto& request = subjectRequests_.at(r);
        professorRequests_[request.Professor()].insert(r);
        for(std::size_t g : request.Groups())
            groupRequests_[g].insert(r);
    }
}

bool ScheduleData::Intersects(std::size_t lhsSubjectRequest, std::size_t rhsSubjectRequest) const
{
    return intersectionsTable_.get_bit(lhsSubjectRequest, rhsSubjectRequest);
}

const SubjectRequest& ScheduleData::SubjectRequestAtID(std::size_t subjectRequestID) const
{
    auto it = std::ranges::lower_bound(subjectRequests_, subjectRequestID, {}, &SubjectRequest::ID);
    if(it == subjectRequests_.end() || it->ID() != subjectRequestID)
        throw std::out_of_range("Subject request with ID=" + std::to_string(subjectRequestID)
                                + " is not found!");

    return *it;
}

SubjectRequest& ScheduleData::SubjectRequestAtID(std::size_t subjectRequestID)
{
    return const_cast<SubjectRequest&>(
        const_cast<const ScheduleData*>(this)->SubjectRequestAtID(subjectRequestID));
}

std::size_t ScheduleData::IndexOfSubjectRequestWithID(std::size_t subjectRequestID) const
{
    auto it = std::ranges::lower_bound(subjectRequests_, subjectRequestID, {}, &SubjectRequest::ID);
    if(it == subjectRequests_.end() || it->ID() != subjectRequestID)
        throw std::out_of_range("Subject request with ID=" + std::to_string(subjectRequestID)
                                + " is not found!");

    return std::distance(subjectRequests_.begin(), it);
}

bool ScheduleData::IsInBlock(std::size_t subjectRequestIndex) const
{
    return requestsBlocks_.count(subjectRequestIndex) != 0;
}

const SubjectsBlock* ScheduleData::FindBlockByRequestIndex(std::size_t subjectRequestIndex) const
{
    auto it = requestsBlocks_.find(subjectRequestIndex);
    if(it == requestsBlocks_.end())
        return nullptr;

    return &blocks_.at(it->second);
}


std::vector<std::size_t> SelectBlockFirstLessons(const std::vector<SubjectRequest>& requests,
                                                 const std::vector<std::size_t>& block)
{
    assert(std::ranges::is_sorted(requests, {}, &SubjectRequest::ID));

    std::vector<std::size_t> blockFirstLessons;
    for(std::size_t lesson : requests.at(block.front()).Lessons())
    {
        bool matches = true;
        for(std::size_t i = 1, l = lesson; i < block.size(); ++i, ++l)
        {
            const auto& lessons = requests.at(block.at(i)).Lessons();
            matches = std::binary_search(lessons.begin(), lessons.end(), l);
            if(!matches)
                break;
        }

        if(matches)
            blockFirstLessons.emplace_back(lesson);
    }

    blockFirstLessons.shrink_to_fit();
    return blockFirstLessons;
}

SubjectsBlock ToSubjectsBlock(const std::vector<SubjectRequest>& requests,
                              const std::vector<std::size_t>& requestIDs)
{
    assert(std::ranges::is_sorted(requests, {}, &SubjectRequest::ID));

    std::vector<std::size_t> block(requestIDs.size());
    std::transform(requestIDs.begin(),
                   requestIDs.end(),
                   block.begin(),
                   [&](std::size_t id)
                   {
                       auto it = std::ranges::lower_bound(requests, id, {}, &SubjectRequest::ID);
                       if(it == requests.end())
                           throw std::invalid_argument("Subject request was not found");

                       return std::distance(requests.begin(), it);
                   });

    auto lessons = SelectBlockFirstLessons(requests, block);
    return SubjectsBlock{std::move(block), std::move(lessons)};
}

std::vector<SubjectsBlock> ToSubjectsBlocks(const std::vector<SubjectRequest>& requests,
                                            const std::vector<std::vector<std::size_t>>& blocksIds)
{
    std::vector<SubjectsBlock> subjectsBlocks;
    subjectsBlocks.reserve(blocksIds.size());
    std::transform(blocksIds.begin(),
                   blocksIds.end(),
                   std::back_inserter(subjectsBlocks),
                   [&](const std::vector<std::size_t>& b) { return ToSubjectsBlock(requests, b); });

    return subjectsBlocks;
}

BitIntersectionsMatrix FillIntersectionsMatrix(const std::vector<SubjectRequest>& requests)
{
    BitIntersectionsMatrix mtx(requests.size());
    for(std::size_t i = 1; i < requests.size(); ++i)
    {
        for(std::size_t j = 0; j < i; ++j)
        {
            const auto& thisRequest = requests[i];
            const auto& otherRequest = requests[j];
            mtx.set_bit(
                i,
                j,
                thisRequest.Professor() == otherRequest.Professor()
                    || set_intersects(thisRequest.Groups(), otherRequest.Groups())
                    || (thisRequest.Classrooms().size() == 1
                        && otherRequest.Classrooms().size() == 1
                        && thisRequest.Classrooms().front() == otherRequest.Classrooms().front()));
        }
    }

    return mtx;
}

std::unordered_map<std::size_t, std::size_t> FillRequestsBlocksTable(const std::vector<SubjectsBlock>& blocks)
{
    std::unordered_map<std::size_t, std::size_t> requestsBlocks;
    for(std::size_t b = 0; b < blocks.size(); ++b)
    {
        const SubjectsBlock& block = blocks.at(b);
        for(std::size_t r : block.Requests())
        {
            if(requestsBlocks.count(r) != 0)
                throw std::invalid_argument("One subject request found in several blocks");

            requestsBlocks.emplace(r, b);
        }
    }
    return requestsBlocks;
}

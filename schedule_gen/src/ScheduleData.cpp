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
    std::ranges::sort(groups_);
    groups_.erase(std::unique(groups_.begin(), groups_.end()), groups_.end());

    std::ranges::sort(lessons_);
    lessons_.erase(std::unique(lessons_.begin(), lessons_.end()), lessons_.end());

    if(lessons_.empty())
        lessons_ = AllLessons();

    std::ranges::sort(classrooms_);
    classrooms_.erase(std::unique(classrooms_.begin(), classrooms_.end()), classrooms_.end());
}

const std::vector<std::size_t>& SubjectRequest::Lessons() const
{
    return lessons_.empty() ? AllLessons() : lessons_;
}

void SubjectRequest::SetLessons(std::vector<std::size_t> lessons)
{
    std::ranges::sort(lessons);
    lessons.erase(std::unique(lessons.begin(), lessons.end()), lessons.end());
    lessons_ = std::move(lessons);
}


ScheduleData::ScheduleData(std::vector<SubjectRequest> subjectRequests,
                           std::vector<SubjectsBlock> blocks)
    : subjectRequests_(std::move(subjectRequests))
    , intersectionsTable_(subjectRequests_.size())
    , blocks_(std::move(blocks))
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

    for(std::size_t b = 0; b < blocks_.size(); ++b)
    {
        const SubjectsBlock& block = blocks_.at(b);
        for(std::size_t r : block.Requests())
            requestsBlocks_.emplace(r, b);
    }

    for(std::size_t i = 1; i < subjectRequests_.size(); ++i)
    {
        for(std::size_t j = 0; j < i; ++j)
        {
            const auto& thisRequest = subjectRequests_[i];
            const auto& otherRequest = subjectRequests_[j];
            intersectionsTable_.set_bit(
                i,
                j,
                thisRequest.Professor() == otherRequest.Professor()
                    || set_intersects(thisRequest.Groups(), otherRequest.Groups())
                    || (thisRequest.Classrooms().size() == 1
                        && otherRequest.Classrooms().size() == 1
                        && thisRequest.Classrooms().front() == otherRequest.Classrooms().front()));
        }
    }

    auto it = remove_duplicates(blocks_.begin(),
                                blocks_.end(),
                                [](const SubjectsBlock& lhs, const SubjectsBlock& rhs) {
                                    return unsorted_set_intersects(lhs.Requests(), rhs.Requests());
                                });

    if(it != blocks_.end())
        throw std::invalid_argument("One subject request found in several blocks");
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

    return SubjectsBlock{block, SelectBlockFirstLessons(requests, block)};
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

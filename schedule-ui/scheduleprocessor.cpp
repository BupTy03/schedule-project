#include "scheduleprocessor.hpp"
#include <cassert>


ScheduleProcessor::ScheduleProcessor(std::unique_ptr<ScheduleGenerator> generator,
                                     QObject* parent)
    : QObject(parent)
    , generator_(std::move(generator))
    , data_()
    , result_()
{
    assert(generator_ != nullptr);
}

void ScheduleProcessor::start()
{
    result_ = std::make_shared<ScheduleResult>(generator_->Generate(*data_));
    emit done();
}

std::shared_ptr<ScheduleResult> ScheduleProcessor::result() const
{
    return result_;
}

void ScheduleProcessor::setData(std::shared_ptr<ScheduleData> data)
{
    data_ = data;
}
std::shared_ptr<ScheduleData> ScheduleProcessor::data() const
{
    return data_;
}

#pragma once
#include "ScheduleGenerator.hpp"

#include <QObject>
#include <memory>


class ScheduleProcessor : public QObject
{
    Q_OBJECT
public:
    explicit ScheduleProcessor(std::unique_ptr<ScheduleGenerator> generator,
                               QObject* parent = nullptr);

    void setData(std::shared_ptr<ScheduleData> data);
    [[nodiscard]] std::shared_ptr<ScheduleResult> result() const;

signals:
    void done();

public slots:
    void start();

private:
    std::unique_ptr<ScheduleGenerator> generator_;
    std::shared_ptr<ScheduleData> data_;
    std::shared_ptr<ScheduleResult> result_;
};

#pragma once
#include <QHeaderView>
#include <memory>


class QAbstractItemModel;

class HierarchicalHeaderView : public QHeaderView
{
    Q_OBJECT
public:
    explicit HierarchicalHeaderView(Qt::Orientation orientation,
                                    std::unique_ptr<QAbstractItemModel> headerModel,
                                    QWidget* parent = 0);

protected:
    void paintSection(QPainter* painter, const QRect& rect, int logicalIndex) const override;
    QSize sectionSizeFromContents(int logicalIndex) const override;

private slots:
    void onSectionResized(int logicalIndex);

private:
    QModelIndex leafIndex(int sectionIndex) const;
    QStyleOptionHeader styleOptionForCell(int logicalIndex) const;

    QSize cellSize(const QModelIndex& leafIndex,
                   const QStyleOptionHeader& styleOptions) const;

    int currentCellWidth(const QModelIndex& parentIndex,
                         const QModelIndex& leafIndex,
                         int sectionIndex) const;

    int currentCellLeft(const QModelIndex& parentIndex,
                        const QModelIndex& leafIndex,
                        int sectionIndex,
                        int left) const;

    void paintHeader(QPainter& painter,
                     const QRect& sectionRect,
                     int logicalLeafIndex,
                     const QModelIndex& leafIndex) const;

    QRect calculateHeaderRect(const QModelIndex& cellIndex,
                              const QModelIndex& leafIndex,
                              int logicalLeafIndex,
                              const QStyleOptionHeader& uniopt,
                              const QRect& sectionRect,
                              const QPoint& pos) const;

    QPoint paintHeaderCell(QPainter& painter,
                           const QModelIndex& cellIndex,
                           QStyleOptionHeader& uniopt,
                           const QRect& r) const;

private:
    std::unique_ptr<QAbstractItemModel> headerModel_;
};

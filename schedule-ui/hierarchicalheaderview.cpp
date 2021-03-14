#include "hierarchicalheaderview.hpp"
#include "utils.hpp"

#include <QAbstractItemModel>
#include <QPainter>


static QModelIndex findRootIndex(QModelIndex index)
{
    while (index.parent().isValid())
        index = index.parent();

    return index;
}

static QModelIndexList parentIndexes(QModelIndex index)
{
    QModelIndexList indexes;
    while (index.isValid())
    {
        indexes.push_front(index);
        index = index.parent();
    }
    return indexes;
}

static QModelIndex findLeaf(const QModelIndex& currentIndex, int sectionIndex, int& currentLeafIndex)
{
    if (!currentIndex.isValid())
        return {};

    const int childCount = currentIndex.model()->columnCount(currentIndex);
    if (childCount > 0)
    {
        for (int i = 0; i < childCount; ++i)
        {
            QModelIndex res =
                findLeaf(currentIndex.child(0, i), sectionIndex, currentLeafIndex);
            if (res.isValid())
                return res;
        }
    }
    else
    {
        ++currentLeafIndex;
        if (currentLeafIndex == sectionIndex)
            return currentIndex;
    }

    return {};
}

// Найти листья с учётом текущего индекса
static QModelIndexList searchLeafs(const QModelIndex& currentIndex)
{
    if (!currentIndex.isValid())
        return {};

    QModelIndexList res;
    const int childCount = currentIndex.model()->columnCount(currentIndex);
    if (childCount > 0)
    {
        for (int i = 0; i < childCount; ++i)
            res += searchLeafs(currentIndex.child(0, i));
    }
    else
    {
        res.push_back(currentIndex);
    }

    return res;
}

// Найти листья не считая текущий индекс
static QModelIndexList leafs(const QModelIndex& searchedIndex)
{
    if (!searchedIndex.isValid())
        return {};

    QModelIndexList leafs;
    const int childCount = searchedIndex.model()->columnCount(searchedIndex);
    for (int i = 0; i < childCount; ++i)
        leafs += searchLeafs(searchedIndex.child(0, i));

    return leafs;
}

static void applyIndexBrushes(QStyleOptionHeader& opt,
                              const QModelIndex& index)
{
    QVariant foregroundBrush = index.data(Qt::ForegroundRole);
    if (foregroundBrush.canConvert<QBrush>())
        opt.palette.setBrush(QPalette::ButtonText,
                             qvariant_cast<QBrush>(foregroundBrush));

    QVariant backgroundBrush = index.data(Qt::BackgroundRole);
    if (backgroundBrush.canConvert<QBrush>())
    {
        opt.palette.setBrush(QPalette::Button,
                             qvariant_cast<QBrush>(backgroundBrush));
        opt.palette.setBrush(QPalette::Window,
                             qvariant_cast<QBrush>(backgroundBrush));
    }
}


HierarchicalHeaderView::HierarchicalHeaderView(Qt::Orientation orientation,
                                               std::unique_ptr<QAbstractItemModel> headerModel,
                                               QWidget* parent)
    : QHeaderView(orientation, parent)
    , headerModel_(std::move(headerModel))
{
    QObject::connect(this, &QHeaderView::sectionResized, this, &HierarchicalHeaderView::onSectionResized);
}

QPoint HierarchicalHeaderView::paintHeaderCell(QPainter& painter,
                                               const QModelIndex& cellIndex,
                                               QStyleOptionHeader& uniopt,
                                               const QRect& r) const
{
    QPainterStateSaver _(painter);

    uniopt.text = cellIndex.data(Qt::DisplayRole).toString();
    uniopt.rect = r;
    if (cellIndex.data(Qt::UserRole).isValid())
    {
        style()->drawControl(QStyle::CE_HeaderSection, &uniopt, &painter, this);
        QMatrix m;
        m.rotate(-90);
        painter.setWorldMatrix(m, true);
        QRect newRect(0, 0, r.height(), r.width());
        newRect.moveCenter(QPoint(-r.center().y(), r.center().x()));
        uniopt.rect = newRect;
        style()->drawControl(QStyle::CE_HeaderLabel, &uniopt, &painter, this);
    }
    else
    {
        style()->drawControl(QStyle::CE_Header, &uniopt, &painter, this);
    }

    if (orientation() == Qt::Orientation::Horizontal)
        return QPoint(r.left(), r.top() + r.height());
    else
        return QPoint(r.left() + r.width(), r.top());
}

QRect HierarchicalHeaderView::calculateHeaderRect(
    const QModelIndex& cellIndex,
    const QModelIndex& leafIndex,
    int logicalLeafIndex,
    const QStyleOptionHeader& uniopt,
    const QRect& sectionRect,
    const QPoint& pos) const
{
    if (orientation() == Qt::Orientation::Vertical)
    {
        const int width = (cellIndex == leafIndex)
                              ? sectionRect.width() - pos.x()
                              : cellSize(cellIndex, uniopt).width();
        const int top = currentCellLeft(cellIndex, leafIndex, logicalLeafIndex, sectionRect.top());
        const int height = currentCellWidth(cellIndex, leafIndex, logicalLeafIndex);

        return QRect(pos.x(), top, width, height);
    }
    else
    {
        const int height = (cellIndex == leafIndex)
                               ? sectionRect.height() - pos.y()
                               : cellSize(cellIndex, uniopt).height();
        const int left = currentCellLeft(cellIndex, leafIndex, logicalLeafIndex, sectionRect.left());
        const int width = currentCellWidth(cellIndex, leafIndex, logicalLeafIndex);

        return QRect(left, pos.y(), width, height);
    }
}

void HierarchicalHeaderView::paintHeader(QPainter& painter,
                                         const QRect& sectionRect,
                                         int logicalLeafIndex,
                                         const QModelIndex& leafIndex) const
{
    QPainterBrushOriginSaver _(painter);

    const QStyleOptionHeader styleOptions = styleOptionForCell(logicalLeafIndex);
    QPoint pos = sectionRect.topLeft();
    const QModelIndexList indexes = parentIndexes(leafIndex);
    for (int i = 0; i < indexes.size(); ++i)
    {
        QStyleOptionHeader realStyleOptions(styleOptions);
        if (i < indexes.size() - 1 && (realStyleOptions.state.testFlag(QStyle::State_Sunken) || realStyleOptions.state.testFlag(QStyle::State_On)))
        {
            QStyle::State t(QStyle::State_Sunken | QStyle::State_On);
            realStyleOptions.state &= (~t);
        }

        const QModelIndex cellIndex = indexes.at(i);

        QStyleOptionHeader uniopt(realStyleOptions);
        applyIndexBrushes(uniopt, cellIndex);

        const auto r = calculateHeaderRect(cellIndex, leafIndex, logicalLeafIndex, uniopt, sectionRect, pos);
        pos = paintHeaderCell(painter, cellIndex, uniopt, r);
    }
}

QStyleOptionHeader
    HierarchicalHeaderView::styleOptionForCell(int logicalInd) const
{
    QStyleOptionHeader opt;
    initStyleOption(&opt);
    if (window()->isActiveWindow())
        opt.state |= QStyle::State_Active;

    opt.textAlignment = Qt::AlignCenter;
    opt.iconAlignment = Qt::AlignVCenter;
    opt.section = logicalInd;

    int visual = visualIndex(logicalInd);

    if (count() == 1)
        opt.position = QStyleOptionHeader::OnlyOneSection;
    else
    {
        if (visual == 0)
            opt.position = QStyleOptionHeader::Beginning;
        else
            opt.position = (visual == count() - 1 ? QStyleOptionHeader::End
                                                  : QStyleOptionHeader::Middle);
    }

    if (isEnabled())
    {
        if (highlightSections() && selectionModel())
        {
            if (orientation() == Qt::Horizontal)
            {
                if (selectionModel()->columnIntersectsSelection(logicalInd,
                                                                rootIndex()))
                    opt.state |= QStyle::State_On;
                if (selectionModel()->isColumnSelected(logicalInd, rootIndex()))
                    opt.state |= QStyle::State_Sunken;
            }
            else
            {
                if (selectionModel()->rowIntersectsSelection(logicalInd, rootIndex()))
                    opt.state |= QStyle::State_On;
                if (selectionModel()->isRowSelected(logicalInd, rootIndex()))
                    opt.state |= QStyle::State_Sunken;
            }
        }
    }
    if (selectionModel())
    {
        bool previousSelected = false;
        if (orientation() == Qt::Horizontal)
            previousSelected = selectionModel()->isColumnSelected(
                logicalIndex(visual - 1), rootIndex());
        else
            previousSelected = selectionModel()->isRowSelected(
                logicalIndex(visual - 1), rootIndex());
        bool nextSelected = false;
        if (orientation() == Qt::Horizontal)
            nextSelected = selectionModel()->isColumnSelected(
                logicalIndex(visual + 1), rootIndex());
        else
            nextSelected = selectionModel()->isRowSelected(logicalIndex(visual + 1),
                                                           rootIndex());
        if (previousSelected && nextSelected)
            opt.selectedPosition = QStyleOptionHeader::NextAndPreviousAreSelected;
        else
        {
            if (previousSelected)
                opt.selectedPosition = QStyleOptionHeader::PreviousIsSelected;
            else
            {
                if (nextSelected)
                    opt.selectedPosition = QStyleOptionHeader::NextIsSelected;
                else
                    opt.selectedPosition = QStyleOptionHeader::NotAdjacent;
            }
        }
    }
    return opt;
}

QSize HierarchicalHeaderView::sectionSizeFromContents(int logicalIndex) const
{
    QModelIndex currLeafIndex = leafIndex(logicalIndex);
    if (!currLeafIndex.isValid())
        return QHeaderView::sectionSizeFromContents(logicalIndex);

    const QStyleOptionHeader styleOption = styleOptionForCell(logicalIndex);
    QSize s = cellSize(currLeafIndex, styleOption);
    currLeafIndex = currLeafIndex.parent();
    while (currLeafIndex.isValid())
    {
        if (orientation() == Qt::Horizontal)
            s.rheight() += cellSize(currLeafIndex, styleOption).height();
        else
            s.rwidth() += cellSize(currLeafIndex, styleOption).width();

        currLeafIndex = currLeafIndex.parent();
    }
    return s;
}

void HierarchicalHeaderView::paintSection(QPainter* painter, const QRect& rect, int logicalIndex) const
{
    assert(painter != nullptr);
    assert(rect.isValid());

    QModelIndex leaf = leafIndex(logicalIndex);
    if (leaf.isValid())
    {
        paintHeader(*painter, rect, logicalIndex, leaf);
    }
    else
    {
        QHeaderView::paintSection(painter, rect, logicalIndex);
    }
}

void HierarchicalHeaderView::onSectionResized(int logicalIndex)
{
    if (isSectionHidden(logicalIndex))
        return;

    QModelIndex leaf = leafIndex(logicalIndex);
    if (!leaf.isValid())
        return;

    QModelIndexList leafsList(leafs(findRootIndex(leaf)));
    for (int n = leafsList.indexOf(leaf); n > 0; --n)
    {
        --logicalIndex;

        const int w = viewport()->width();
        const int h = viewport()->height();
        const int pos = sectionViewportPosition(logicalIndex);
        QRect r(pos, 0, w - pos, h);
        if (orientation() == Qt::Horizontal)
        {
            if (isRightToLeft())
                r.setRect(0, 0, pos + sectionSize(logicalIndex), h);
        }
        else
            r.setRect(0, pos, w, h - pos);

        viewport()->update(r.normalized());
    }
}

QModelIndex HierarchicalHeaderView::leafIndex(int sectionIndex) const
{
    int curentLeafIndex = -1;
    for (int i = 0; i < headerModel_->columnCount(); ++i)
    {
        const QModelIndex res = findLeaf(headerModel_->index(0, i), sectionIndex, curentLeafIndex);
        if (res.isValid())
            return res;
    }

    return {};
}

QSize HierarchicalHeaderView::cellSize(const QModelIndex& leafIndex,
                                       const QStyleOptionHeader& styleOptions) const
{
    QSize res;
    QVariant variant(leafIndex.data(Qt::SizeHintRole));
    if (variant.isValid())
        res = qvariant_cast<QSize>(variant);

    QFont fnt = font();
    QVariant var(leafIndex.data(Qt::FontRole));
    if (var.isValid() && var.canConvert<QFont>())
        fnt = qvariant_cast<QFont>(var);

    fnt.setBold(true);
    const QFontMetrics fm(fnt);
    QSize size = fm.size(0, leafIndex.data(Qt::DisplayRole).toString());
    if (leafIndex.data(Qt::UserRole).isValid())
        size.transpose();

    const QSize decorationsSize(style()->sizeFromContents(
        QStyle::CT_HeaderSection, &styleOptions, QSize(), this));
    const QSize emptyTextSize(fm.size(0, ""));
    return res.expandedTo(size + decorationsSize - emptyTextSize);
}

int HierarchicalHeaderView::currentCellWidth(const QModelIndex& parentIndex,
                                             const QModelIndex& leafIndex,
                                             int sectionIndex) const
{
    // ширина родительской ячейки рассчитывается как общая ширина всех листовых
    // ячеек
    const QModelIndexList leafsList = leafs(parentIndex);
    if (leafsList.empty())
        return sectionSize(sectionIndex);

    int width = 0;
    int firstLeafSectionIndex = sectionIndex - leafsList.indexOf(leafIndex);
    for (int i = 0; i < leafsList.size(); ++i)
        width += sectionSize(firstLeafSectionIndex + i);

    return width;
}

int HierarchicalHeaderView::currentCellLeft(const QModelIndex& parentIndex,
                                            const QModelIndex& leafIndex,
                                            int sectionIndex,
                                            int left) const
{
    // начальная позиция родительского индекса рассчитывается исходя из позиции
    // листового индекса
    const QModelIndexList leafsList = leafs(parentIndex);
    if (leafsList.empty())
        return left;

    int n = leafsList.indexOf(leafIndex);
    const int firstLeafSectionIndex = sectionIndex - n;
    --n;
    for (; n >= 0; --n)
        left -= sectionSize(firstLeafSectionIndex + n);

    return left;
}

/****************************************************************************
 **
 ** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
 ** All rights reserved.
 ** Contact: Nokia Corporation (qt-info@nokia.com)
 **
 ** This file is part of the QtLocation module of the Qt Toolkit.
 **
 ** $QT_BEGIN_LICENSE:LGPL$
 ** GNU Lesser General Public License Usage
 ** This file may be used under the terms of the GNU Lesser General Public
 ** License version 2.1 as published by the Free Software Foundation and
 ** appearing in the file LICENSE.LGPL included in the packaging of this
 ** file. Please review the following information to ensure the GNU Lesser
 ** General Public License version 2.1 requirements will be met:
 ** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
 **
 ** In addition, as a special exception, Nokia gives you certain additional
 ** rights. These rights are described in the Nokia Qt LGPL Exception
 ** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
 **
 ** GNU General Public License Usage
 ** Alternatively, this file may be used under the terms of the GNU General
 ** Public License version 3.0 as published by the Free Software Foundation
 ** and appearing in the file LICENSE.GPL included in the packaging of this
 ** file. Please review the following information to ensure the GNU General
 ** Public License version 3.0 requirements will be met:
 ** http://www.gnu.org/copyleft/gpl.html.
 **
 ** Other Usage
 ** Alternatively, this file may be used in accordance with the terms and
 ** conditions contained in a signed written agreement between you and Nokia.
 **
 **
 **
 **
 **
 ** $QT_END_LICENSE$
 **
 ****************************************************************************/

#include "qdeclarativepolylinemapitem_p.h"
#include "qdeclarativegeomapquickitem_p.h"
#include <QDeclarativeInfo>
#include <QPainter>

static QPainterPath createPath(const Map& map, const QList<QGeoCoordinate> &path, qreal& w,
                               qreal& h)
{
    QPainterPath painter;

    qreal minX, maxX, minY, maxY;
    //TODO: dateline handling

    for (int i = 0; i < path.size(); ++i) {

        const QGeoCoordinate &coord = path.at(i);

        if (!coord.isValid())
            continue;

        QPointF point = map.coordinateToScreenPosition(coord, false);

        if (i == 0) {
            minX = point.x();
            maxX = point.x();
            minY = point.y();
            maxY = point.y();
            painter.moveTo(point);
        } else {
            minX = qMin(point.x(), minX);
            maxX = qMax(point.x(), maxX);
            minY = qMin(point.y(), minY);
            maxY = qMax(point.y(), maxY);
            painter.lineTo(point);
        }

    }

    painter.translate(-minX, -minY);

    w = maxX - minX;
    h = maxY - minY;

    return painter;
}

QDeclarativePolylineMapItem::QDeclarativePolylineMapItem(QQuickItem *parent) :
        QDeclarativeGeoMapItemBase(parent), quickItem_(new QDeclarativeGeoMapQuickItem(this)), polylineMapPaintedItem_(
            new PolylineMapPaintedItem(quickItem_)), initialized_(false)
{
}

QDeclarativePolylineMapItem::~QDeclarativePolylineMapItem()
{
}

void QDeclarativePolylineMapItem::componentComplete()
{
    initialized_ = true;
}

QDeclarativeListProperty<QDeclarativeCoordinate> QDeclarativePolylineMapItem::declarativePath()
{
    return QDeclarativeListProperty<QDeclarativeCoordinate>(this, 0, path_append, path_count,
            path_at, path_clear);
}

void QDeclarativePolylineMapItem::path_append(
    QDeclarativeListProperty<QDeclarativeCoordinate> *property, QDeclarativeCoordinate *coordinate)
{
    QDeclarativePolylineMapItem* item = qobject_cast<QDeclarativePolylineMapItem*>(
                                            property->object);
    item->path_.append(coordinate);
    QList<QGeoCoordinate> p = item->polylineMapPaintedItem_->path();
    p.append(coordinate->coordinate());
    item->polylineMapPaintedItem_->setPath(p);
    if (item->initialized_)
        emit item->pathChanged();
}

int QDeclarativePolylineMapItem::path_count(
    QDeclarativeListProperty<QDeclarativeCoordinate> *property)
{
    return qobject_cast<QDeclarativePolylineMapItem*>(property->object)->path_.count();
}

QDeclarativeCoordinate* QDeclarativePolylineMapItem::path_at(
    QDeclarativeListProperty<QDeclarativeCoordinate> *property, int index)
{
    return qobject_cast<QDeclarativePolylineMapItem*>(property->object)->path_.at(index);
}

void QDeclarativePolylineMapItem::path_clear(
    QDeclarativeListProperty<QDeclarativeCoordinate> *property)
{
    QDeclarativePolylineMapItem* item = qobject_cast<QDeclarativePolylineMapItem*>(
                                            property->object);
    qDeleteAll(item->path_);
    item->path_.clear();
    item->polylineMapPaintedItem_->setPath(QList<QGeoCoordinate>());
    if (item->initialized_)
        emit item->pathChanged();
}

void QDeclarativePolylineMapItem::addCoordinate(QDeclarativeCoordinate* coordinate)
{
    path_.append(coordinate);
    QList<QGeoCoordinate> path = polylineMapPaintedItem_->path();
    path.append(coordinate->coordinate());
    polylineMapPaintedItem_->setPath(path);
    emit pathChanged();
}

void QDeclarativePolylineMapItem::removeCoordinate(QDeclarativeCoordinate* coordinate)
{
    int index = path_.lastIndexOf(coordinate);

    if (index == -1) {
        qmlInfo(this) << tr("Coordinate does not belong to PolylineMapItem.");
        return;
    }

    QList<QGeoCoordinate> path = polylineMapPaintedItem_->path();

    if (path.count() < index + 1) {
        qmlInfo(this) << tr("Coordinate does not belong to PolylineMapItem.");
        return;
    }
    path.removeAt(index);
    polylineMapPaintedItem_->setPath(path);
    path_.removeAt(index);
    emit pathChanged();
}

void QDeclarativePolylineMapItem::update()
{
    if (!map())
        return;

    quickItem_->setCoordinate(
        new QDeclarativeCoordinate(polylineMapPaintedItem_->quickItemCoordinate()));
    quickItem_->setAnchorPoint(polylineMapPaintedItem_->quickItemAnchorPoint());

    if (quickItem_->sourceItem() == 0) {
        QObject::connect(map(), SIGNAL(cameraDataChanged(CameraData)), this,
                         SLOT(handleCameraDataChanged(CameraData)));
        polylineMapPaintedItem_->setMap(map());
        quickItem_->setMap(quickMap(), map());
        quickItem_->setSourceItem(polylineMapPaintedItem_);
    }
}

void QDeclarativePolylineMapItem::handleCameraDataChanged(const CameraData& cameraData)
{
    polylineMapPaintedItem_->setZoomLevel(cameraData.zoomFactor());
    update();
}

QColor QDeclarativePolylineMapItem::color() const
{
    return color_;
}

void QDeclarativePolylineMapItem::setColor(const QColor &color)
{
    if (color_ == color)
        return;

    color_ = color;
    polylineMapPaintedItem_->setPen(color);
    emit colorChanged(color_);
}

//////////////////////////////////////////////////////////////////////

PolylineMapPaintedItem::PolylineMapPaintedItem(QQuickItem *parent) :
        QQuickPaintedItem(parent), map_(0), zoomLevel_(-1), initialized_(false)
{
    setAntialiasing(true);
    connect(this, SIGNAL(xChanged()), this, SLOT(update()));
    connect(this, SIGNAL(yChanged()), this, SLOT(update()));
}

PolylineMapPaintedItem::~PolylineMapPaintedItem()
{
}

void PolylineMapPaintedItem::setMap(Map* map)
{
    map_ = map;
}

void PolylineMapPaintedItem::setZoomLevel(qreal zoomLevel)
{
    if (zoomLevel_ == zoomLevel)
        return;

    zoomLevel_ = zoomLevel;
    updateGeometry();
}

qreal PolylineMapPaintedItem::zoomLevel() const
{
    return zoomLevel_;
}

void PolylineMapPaintedItem::setPath(const QList<QGeoCoordinate>& path)
{
    coordPath_ = path;
    updateGeometry();
}

QList<QGeoCoordinate> PolylineMapPaintedItem::path() const
{
    return coordPath_;
}

QGeoCoordinate PolylineMapPaintedItem::quickItemCoordinate() const
{
    return quickItemCoordinate_;
}

QPointF PolylineMapPaintedItem::quickItemAnchorPoint() const
{
    return quickItemAnchorPoint_;
}

void PolylineMapPaintedItem::setBrush(const QBrush &brush)
{
    brush_ = brush;
}

QBrush PolylineMapPaintedItem::brush() const
{
    return brush_;
}

void PolylineMapPaintedItem::setPen(const QPen &pen)
{
    pen_ = pen;
}

QPen PolylineMapPaintedItem::pen() const
{
    return pen_;
}

void PolylineMapPaintedItem::paint(QPainter *painter)
{
    if (!initialized_)
        return;
    painter->setPen(pen_);
    painter->setBrush(brush_);
    painter->drawPath(path_);
}

void PolylineMapPaintedItem::updateGeometry()
{
    initialized_ = false;

    if (!map_)
        return;

    if (coordPath_.size() == 0)
        return;

    if (zoomLevel_ == -1)
        return;

    QPointF point = map_->coordinateToScreenPosition(coordPath_.at(0), false);

    qreal w = 0;
    qreal h = 0;

    //TODO: optimize essential part
    path_ = createPath(*map_, coordPath_, w, h);

    setWidth(w);
    setHeight(h);
    setContentsSize(QSize(w, h));

    quickItemCoordinate_ = coordPath_.at(0);
    quickItemAnchorPoint_ = path_.pointAtPercent(0);
    initialized_ = true;

    update();
}
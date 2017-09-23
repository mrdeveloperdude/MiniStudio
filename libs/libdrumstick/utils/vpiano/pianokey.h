/*
    Virtual Piano Widget for Qt5
    Copyright (C) 2008-2015, Pedro Lopez-Cabanillas <plcl@users.sf.net>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along 
    with this program; If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef PIANOKEY_H_
#define PIANOKEY_H_

#include "pianodefs.h"
#include <QGraphicsRectItem>
#include <QBrush>

class VPIANO_EXPORT PianoKey : public QGraphicsRectItem
{
public:
    PianoKey(QGraphicsItem * parent = 0 ) 
        : QGraphicsRectItem(parent), m_pressed(false) { }
    PianoKey(const QRectF &rect, const QBrush &brush, const int note); 
    PianoKey(const QRectF &rect, const bool black, const int note);
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *);
    int getNote() const { return m_note; }
    void setBrush(const QBrush& b) { m_brush = b; }
    void setPressedBrush(const QBrush& b) { m_selectedBrush = b; }
    void resetBrush();
    bool isPressed() const { return m_pressed; }
    void setPressed(bool p);
    int getDegree() const { return m_note % 12; }
    int getType() const { return (m_black ? 1 : 0); }

private:
    bool m_pressed;
    QBrush m_selectedBrush;
    QBrush m_brush;
    int m_note;
    bool m_black;
};

#endif /*PIANOKEY_H_*/

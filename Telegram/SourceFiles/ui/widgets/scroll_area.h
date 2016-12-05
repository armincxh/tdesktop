/*
This file is part of Telegram Desktop,
the official desktop version of Telegram messaging app, see https://telegram.org

Telegram Desktop is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

It is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

In addition, as a special exception, the copyright holders give permission
to link the code of portions of this program with the OpenSSL library.

Full license: https://github.com/telegramdesktop/tdesktop/blob/master/LICENSE
Copyright (c) 2014-2016 John Preston, https://desktop.telegram.org
*/
#pragma once

#include "styles/style_widgets.h"

namespace Ui {

enum class TouchScrollState {
	Manual, // Scrolling manually with the finger on the screen
	Auto, // Scrolling automatically
	Acceleration // Scrolling automatically but a finger is on the screen
};

class ScrollArea;

class ScrollShadow : public QWidget {
	Q_OBJECT

public:

	ScrollShadow(ScrollArea *parent, const style::FlatScroll *st);

	void paintEvent(QPaintEvent *e);

public slots:

	void changeVisibility(bool shown);

private:

	const style::FlatScroll *_st;

};

class ScrollBar : public QWidget {
	Q_OBJECT

public:

	ScrollBar(ScrollArea *parent, bool vertical, const style::FlatScroll *st);

	void recountSize();

	void paintEvent(QPaintEvent *e);
	void enterEvent(QEvent *e);
	void leaveEvent(QEvent *e);
	void mouseMoveEvent(QMouseEvent *e);
	void mousePressEvent(QMouseEvent *e);
	void mouseReleaseEvent(QMouseEvent *e);
	void resizeEvent(QResizeEvent *e);

	void step_appearance(float64 ms, bool timer);

	void hideTimeout(TimeMs dt);

public slots:

	void onValueChanged();
	void updateBar(bool force = false);
	void onHideTimer();

signals:

	void topShadowVisibility(bool);
	void bottomShadowVisibility(bool);

private:

	ScrollArea *area();
	const style::FlatScroll *_st;

	bool _vertical;
	bool _over, _overbar, _moving;
	bool _topSh, _bottomSh;

	QPoint _dragStart;
	QScrollBar *_connected;

	int32 _startFrom, _scrollMax;

	TimeMs _hideIn;
	QTimer _hideTimer;

	anim::fvalue a_bgOver, a_barOver, a_fullOpacity;
	Animation _a_appearance;

	QRect _bar;
};

class SplittedWidget : public TWidget {
	Q_OBJECT

public:
	SplittedWidget(QWidget *parent) : TWidget(parent) {
		setAttribute(Qt::WA_OpaquePaintEvent);
	}
	void setHeight(int32 newHeight) {
		resize(width(), newHeight);
		emit resizeOther();
	}
	void update(int x, int y, int w, int h) {
		update(QRect(x, y, w, h));
	}
	void update(const QRect&);
	void update(const QRegion&);
	void rtlupdate(const QRect &r) {
		update(myrtlrect(r));
	}
	void rtlupdate(int x, int y, int w, int h) {
		update(myrtlrect(x, y, w, h));
	}

public slots:
	void update() {
		update(0, 0, fullWidth(), height());
	}

signals:
	void resizeOther();
	void updateOther(const QRect&);
	void updateOther(const QRegion&);

protected:
	void paintEvent(QPaintEvent *e) override; // paintEvent done through paintRegion

	int32 otherWidth() const {
		return _otherWidth;
	}
	int32 fullWidth() const {
		return width() + otherWidth();
	}
	virtual void paintRegion(Painter &p, const QRegion &region, bool paintingOther) = 0;

private:
	int32 _otherWidth = 0;
	void setOtherWidth(int32 otherWidth) {
		_otherWidth = otherWidth;
	}
	void resize(int32 w, int32 h) {
		TWidget::resize(w, h);
	}
	friend class ScrollArea;
	friend class SplittedWidgetOther;

};

class SplittedWidgetOther;
class ScrollArea : public QScrollArea {
	Q_OBJECT
	T_WIDGET

public:
	ScrollArea(QWidget *parent, const style::FlatScroll &st = st::defaultFlatScroll, bool handleTouch = true);

	int scrollWidth() const;
	int scrollHeight() const;
	int scrollLeftMax() const;
	int scrollTopMax() const;
	int scrollLeft() const;
	int scrollTop() const;

	void setWidget(QWidget *widget);
	void setOwnedWidget(QWidget *widget);
	QWidget *takeWidget();

	void rangeChanged(int oldMax, int newMax, bool vertical);

	void updateBars();

	bool focusNextPrevChild(bool next) override;
	void setMovingByScrollBar(bool movingByScrollBar);

	bool viewportEvent(QEvent *e) override;
	void keyPressEvent(QKeyEvent *e) override;

	QMargins getMargins() const {
		return QMargins();
	}

	~ScrollArea();

protected:
	bool eventFilter(QObject *obj, QEvent *e) override;

	void resizeEvent(QResizeEvent *e) override;
	void moveEvent(QMoveEvent *e) override;
	void touchEvent(QTouchEvent *e);

	void enterEventHook(QEvent *e);
	void leaveEventHook(QEvent *e);

public slots:
	void scrollToY(int toTop, int toBottom = -1);
	void disableScroll(bool dis);
	void onScrolled();

	void onTouchTimer();
	void onTouchScrollTimer();

	void onResizeOther();
	void onUpdateOther(const QRect&);
	void onUpdateOther(const QRegion&);
	void onVerticalScroll();

signals:
	void scrolled();
	void scrollStarted();
	void scrollFinished();
	void geometryChanged();

protected:
	void scrollContentsBy(int dx, int dy) override;

private:
	bool touchScroll(const QPoint &delta);

	void touchScrollUpdated(const QPoint &screenPos);

	void touchResetSpeed();
	void touchUpdateSpeed();
	void touchDeaccelerate(int32 elapsed);

	bool _disabled = false;
	bool _ownsWidget = false; // if true, the widget is deleted in destructor.
	bool _movingByScrollBar = false;

	const style::FlatScroll &_st;
	ChildWidget<ScrollBar> _horizontalBar, _verticalBar;
	ChildWidget<ScrollShadow> _topShadow, _bottomShadow;
	int _horizontalValue, _verticalValue;

	bool _touchEnabled;
	QTimer _touchTimer;
	bool _touchScroll = false;
	bool _touchPress = false;
	bool _touchRightButton = false;
	QPoint _touchStart, _touchPrevPos, _touchPos;

	TouchScrollState _touchScrollState = TouchScrollState::Manual;
	bool _touchPrevPosValid = false;
	bool _touchWaitingAcceleration = false;
	QPoint _touchSpeed;
	TimeMs _touchSpeedTime = 0;
	TimeMs _touchAccelerationTime = 0;
	TimeMs _touchTime = 0;
	QTimer _touchScrollTimer;

	bool _widgetAcceptsTouch = false;

	friend class SplittedWidgetOther;
	SplittedWidgetOther *_other = nullptr;

};

class SplittedWidgetOther : public TWidget {
public:
	SplittedWidgetOther(ScrollArea *parent) : TWidget(parent) {
	}

protected:
	void paintEvent(QPaintEvent *e) override;

};

} // namespace Ui
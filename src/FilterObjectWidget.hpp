/*
* Software:  SR-Tesseler (Multiscale segmentation of localization-based super-resolution microscopy data with polygons)
*
* File:      FilterObjectWidget.hpp
*
* Copyright: Florian Levet (2010-2019)
*
* License:   GPL v3
*
* Homepage:  http://www.iins.u-bordeaux.fr/team-sibarita-SR-Tesseler
*
*
* SR-Tesseler is a free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 3 of the License, or
* (at your option) any later version, provided that this entire notice
* is included in all copies of any software which is or includes a copy
* or modification of this software and in all copies of the supporting
* documentation for such software.
*
* The algorithms that underlie SR-Tesseler have required considerable
* development. They are described in the original SR-Tesseler paper,
* doi:10.1038/nmeth.3579. If you use SR-Tesseler as part of work towards a
* scientific publication, please include a citation to the original paper.
*
* SR-Tesseler is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*/

#ifndef FilterObjectWidget_h__
#define FilterObjectWidget_h__

#include <QtGui>

#include "HistogramCamera.hpp"

class ObjectInterface;
class Camera2D;

class FilterObjectWidget : public QDockWidget
{
	Q_OBJECT

public:
	/**
	 * \brief Constructor.
	 * \param _data All data loaded (images, detections, trajectories).
	 * \param _camera The camera of the image viewer.
	 * \param _parent Widget parent.
	 * \param _flags Widget flags.
	 */
	FilterObjectWidget( QWidget* _parent = 0, Qt::WFlags _flags = 0 );
	FilterObjectWidget( ObjectInterface *, Camera2D *, QWidget* _parent = 0, Qt::WFlags _flags = 0 );
	/**
	 * \brief Destructor.
	 */
	virtual ~FilterObjectWidget();

	//public slots:
	void updateHistograms( const bool resetHistograms );
	QSize sizeHint() const;

	virtual void setHistogramData( ObjectInterface *, Camera2D * );
	void changeData( ObjectInterface *, Camera2D * );

	inline HistogramCamera * getHistogramCamera() {return m_histoCam;}
	inline bool isLogChecked() const {return m_cboxLog->isChecked();}

protected:
	void closeEvent( QCloseEvent * );

protected slots:
	/**
	 * \brief Set the new intensity boundaries of the detections.
	 * \param _min New min for display.
	 * \param _max New max for display.
	 */
	void setDetectionBoundaries( const double _min, const double _max );
	void isLogChecked( int );
	void isLogChecked( bool );
	void saveHistogramData();

signals:
	void visibleWidget( bool );
	void updateDisplay();

protected:
	HistogramCamera * m_histoCam;
	QLineEdit * lineMinDetection, * lineMaxDetection;
	QCheckBox * m_cboxLog;
	QComboBox * m_lutList, * m_combo;
	QPushButton * m_buttonSave;
};

class FilterDetectionWidget : public FilterObjectWidget{
public:
	FilterDetectionWidget( ObjectInterface *, Camera2D *, QWidget* _parent = 0, Qt::WFlags _flags = 0 );
	~FilterDetectionWidget();

	virtual void setHistogramData( ObjectInterface *, Camera2D * );
};

class FilterVoronoiDiagramWidget : public FilterObjectWidget{
public:
	FilterVoronoiDiagramWidget( ObjectInterface *, Camera2D *, QWidget* _parent = 0, Qt::WFlags _flags = 0 );
	~FilterVoronoiDiagramWidget();

	virtual void setHistogramData( ObjectInterface *, Camera2D * );
};

class FilterVoronoiObjectWidget : public FilterObjectWidget{
public:
	FilterVoronoiObjectWidget( ObjectInterface *, Camera2D *, QWidget* _parent = 0, Qt::WFlags _flags = 0 );
	~FilterVoronoiObjectWidget();

	virtual void setHistogramData( ObjectInterface *, Camera2D * );
};

#endif // FilterObjectWidget_h__
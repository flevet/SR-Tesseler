/*
* Software:  SR-Tesseler (Multiscale segmentation of localization-based super-resolution microscopy data with polygons)
*
* File:      Camera2D.hpp
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

#ifndef Camera2D_h__
#define Camera2D_h__

#include <Windows.h>
#include <QGLWidget>
#include <QScrollBar>
#include <QTabWidget>

#include "Vec2.hpp"
#include "SuperResObject.hpp"

class DetectionSet;
class DetectionCleanerWidget;
class VoronoiWidget;
class RoiManagerWidget;
class MiscQuantificationWidget;

class Camera2D: public QGLWidget{
	Q_OBJECT
public:
	enum Mode
	{
		ModeNone = 0,
		ModeZoom = 1,
		ModeMove = 2,
		RoiDefinition = 3
	};

	Camera2D( QScrollBar *, QScrollBar * );
	~Camera2D();

	void setInteraction( const QString & );
	void zoom( const float, const float, const float );
	void setSuperResObject( SuperResObject * );

	void createVoronoiDiagram( const bool );
	Vec2mf getTrueCoordinates(const int, const int);
	Vec2mf getScreenCoordinates( const double, const double );
	void displayObjectLabels();
	void displayClusterLabels();
	void displayDBSCANLabels();
	void closeAll();

	inline int getZoomedWidth() const {return m_originalImageWidth * m_zoomFactor;}
	inline int getZoomedHeight() const {return m_originalImageHeight * m_zoomFactor;}

	void setDimension( const double, const double );

	void changeBackgroundColor();

	void setPositionZoomParameters(double[5]);
	void getPositionZoomParameters(double[5]);

	void snap(const QString &);
	void exportInVectorialFile(const QString &);

	inline SuperResObject * getSuperResObject() const {return m_superResObj;}
	inline DetectionSet * getDetectionSet() {return m_superResObj->getDetectionSet();}
	inline DetectionSet * getDetectionSetCleaned() {return m_superResObj->getDetectionSetCleaned();}
	inline DetectionCleaner * getDetectionCleaner() {return m_superResObj->getDetectionCleaner();}
	inline WrapperVoronoiDiagram * getVoronoiDiagram(){return m_superResObj->getVoronoiDiagram();}
	inline void setVoronoiDiagram( WrapperVoronoiDiagram * _wrapper ){m_superResObj->setVoronoiDiagram( _wrapper );}
	inline NeuronObjectList & getNeuronObjects() {return m_superResObj->getNeuronObjects();}
	inline const NeuronObjectList & getNeuronObjects() const {return m_superResObj->getNeuronObjects();}
	inline int nbNeuronObjects() const {return m_superResObj->getNeuronObjects().size();}

	inline void setSizePoint( const int _val ){m_sizePoint = _val;}
	inline void setSmoothPoint( const bool _val ){m_pointSmooth = _val;}
	inline void setLineWidth( const int _val ){m_lineWidth = _val;}
	inline void setLineSmooth( const bool _val ){m_lineSmooth = _val;}

	inline const Color4B & getBackgroundColor() const {return m_backColor;}
	inline SuperResObject * getCurrentObject() { return m_superResObj; }

signals:
	void updateSizeViewer();
	void setStatusBarPosition(const QString &);
	void setStatusBarZoom(const QString &);

public slots:
	void moveXScrollbarWithUpdateDisplay( int );
	void moveYScrollbarWithUpdateDisplay( int );
	void toggleGridDisplay();
	void cleanDetections();
	void toggleDisplayObjectLabels( bool );
	void toggleDisplayClusterLabels( bool );
	void toggleDisplayLabelRoi( bool );
	void toggleDisplayDBSCANClusterLabels(bool);
	void loadRois();

protected:
	void initializeGL();
	void paintGL();

	void mousePressEvent(QMouseEvent *);
	void mouseMoveEvent(QMouseEvent *);
	void mouseReleaseEvent(QMouseEvent *);
	void mouseDoubleClickEvent ( QMouseEvent * );
	void wheelEvent( QWheelEvent * );
	void resizeEvent ( QResizeEvent * );
	void determineQStringForStatusBar(const float, const float, const float = -1.f, const float = -1.f);
	void zoomIn();
	void moveImage();
	void moveImage( const int, const int );
	void replaceImageInScrollArea();
	void moveXScrollbar( int );
	void moveYScrollbar( int );
	void displayGrid() const;

protected:
	float m_zoomFactor, m_xv, m_yv, m_widthProjection, m_heightProjection, m_lineWidth;
	bool m_displayGrid, m_button, m_doubleClick, m_pointSmooth, m_lineSmooth, m_displayObjectLabels, m_displayClusterLabels, m_displayDBSCANLabels;
	int m_modeInteraction, m_originalImageWidth, m_originalImageHeight;
	unsigned int m_sizePoint;
	Vec2mf m_interactionStart, m_interactionStop;

	QScrollBar * m_hbar, * m_vbar;

	SuperResObject * m_superResObj;
	QTabWidget * m_tabWidget;
	DetectionCleanerWidget * m_dcw;
	VoronoiWidget * m_voroWidget;
	RoiManagerWidget * m_RoiManagerW;
	MiscQuantificationWidget * m_miscQW;

	Color4B m_backColor;
};
#endif // Camera2D_h__
#ifndef THRESHOLDEDITOR_H
#define THRESHOLDEDITOR_H

#include <QWidget>
#include <QDesktopWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QComboBox>
#include <QCheckBox>
#include <QPushButton>
#include <QFormLayout>
#include <QLineEdit>
#include <QDebug>
#include <QDropEvent>
#include <QMimeData>
#include <QMimeDatabase>

QT_BEGIN_NAMESPACE
namespace Ui { class ThresholdEditor; }
QT_END_NAMESPACE

class ThresholdEditor : public QWidget
{
    Q_OBJECT

public:
    ThresholdEditor(QWidget *parent = nullptr);
    ~ThresholdEditor();

public slots:
    void changed();

protected:
    void resizeEvent(QResizeEvent *event);
    void showEvent(QShowEvent *event);
    void dragEnterEvent(QDragEnterEvent *event);
    void dropEvent(QDropEvent *event);

private:
    Ui::ThresholdEditor *ui;

    QImage m_image;
    QComboBox *m_combo;
    QCheckBox *m_invert;
    QWidget *m_paneG, *m_paneLAB, *m_paneHSV;
    QSlider *m_GMin, *m_GMax; // Grayscale
    QSlider *m_LMin, *m_LMax, *m_AMin, *m_AMax, *m_BMin, *m_BMax; // LAB
    QSlider *m_HMin, *m_HMax, *m_SMin, *m_SMax, *m_VMin, *m_VMax; // HSV
    QLineEdit *m_GOut, *m_LABOut, *m_HSVOut;
    QByteArray m_geometry;
};
#endif // THRESHOLDEDITOR_H

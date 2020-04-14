#include "thresholdeditor.h"
#include "ui_thresholdeditor.h"

extern const uint8_t rb825_table[256];
extern const uint8_t g826_table[256];
extern const int8_t lab_table[196608];
extern const int8_t yuv_table[196608];

static inline int toR5(QRgb value)
{
    return rb825_table[qRed(value)]; // 0:255 -> 0:31
}

static inline int toG6(QRgb value)
{
    return g826_table[qGreen(value)]; // 0:255 -> 0:63
}

static inline int toB5(QRgb value)
{
    return rb825_table[qBlue(value)]; // 0:255 -> 0:31
}

static inline int toRGB565(QRgb value)
{
    int r = toR5(value);
    int g = toG6(value);
    int b = toB5(value);

    return (r << 3) | (g >> 3) | ((g & 0x7) << 13) | (b << 8); // byte reversed.
}

static inline int toGrayscale(QRgb value)
{
    return yuv_table[(toRGB565(value)*3)+0] + 128; // 0:255 -> 0:255
}

static inline int toL(QRgb value)
{
    return lab_table[(toRGB565(value)*3)+0]; // 0:255 -> 0:100
}

static inline int toA(QRgb value)
{
    return lab_table[(toRGB565(value)*3)+1]; // 0:255 -> -128:127
}

static inline int toB(QRgb value)
{
    return lab_table[(toRGB565(value)*3)+2]; // 0:255 -> -128:127
}

ThresholdEditor::ThresholdEditor(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ThresholdEditor)
{
    ui->setupUi(this);
    setAcceptDrops(true);

    QDesktopWidget *desktop = QApplication::desktop();

    setMaximumSize(desktop->screenGeometry().width()/1.2,desktop->screenGeometry().height()/1.2);
    setMinimumSize(desktop->screenGeometry().width()/1.2,desktop->screenGeometry().height()/1.2);


    QVBoxLayout *layout = new QVBoxLayout(ui->widget);

    // Controls //
    {
        QWidget *temp = new QWidget();
        QHBoxLayout *h_layout = new QHBoxLayout(temp);
        h_layout->setMargin(0);
        h_layout->setSpacing(0);

        m_combo = new QComboBox();
        m_combo->addItem(tr("Grayscale"));
        m_combo->addItem(tr("LAB"));
        m_combo->addItem(tr("HSV"));
        h_layout->addWidget(m_combo);
        h_layout->addItem(new QSpacerItem(10, 0));
        connect(m_combo, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &ThresholdEditor::changed);

        m_invert = new QCheckBox(tr("Invert"));
        m_invert->setCheckable(true);
        m_invert->setChecked(false);
        h_layout->addWidget(m_invert);
        connect(m_invert, &QCheckBox::toggled, this, &ThresholdEditor::changed);

        QPushButton *resetButton = new QPushButton(tr("Reset Sliders"));
        h_layout->addWidget(resetButton);

        connect(resetButton, &QPushButton::clicked, this, [this] {
            switch (m_combo->currentIndex())
            {
            case 0:
            {
                m_GMin->setValue(0);
                m_GMax->setValue(255);
            }break;
            case 1:
            {
                m_LMin->setValue(0);
                m_LMax->setValue(100);
                m_AMin->setValue(-128);
                m_AMax->setValue(127);
                m_BMin->setValue(-128);
                m_BMax->setValue(127);
            }break;
            case 2:
            {
                m_HMin->setValue(0);
                m_HMax->setValue(180);
                m_SMin->setValue(0);
                m_SMax->setValue(255);
                m_VMin->setValue(0);
                m_VMax->setValue(255);
            }break;
            }
        });

        QHBoxLayout *t_layout = new QHBoxLayout();
        t_layout->setMargin(0);
        t_layout->addWidget(temp);
        QWidget *t_widget = new QWidget();
        t_widget->setLayout(t_layout);
        layout->addWidget(t_widget);
    }

    // Grayscale //
    {
        m_paneG = new QWidget();
        QFormLayout *v_layout = new QFormLayout(m_paneG);
        v_layout->setMargin(0);

        {
            QWidget *temp = new QWidget();
            QHBoxLayout *h_layout = new QHBoxLayout(temp);
            h_layout->setMargin(0);

            m_GMin = new QSlider(Qt::Horizontal);
            m_GMin->setTickInterval(10);
            m_GMin->setTickPosition(QSlider::TicksBelow);
            m_GMin->setSingleStep(1);
            m_GMin->setPageStep(10);
            m_GMin->setRange(0, 255);
            m_GMin->setValue(0);
            h_layout->addWidget(m_GMin);
            connect(m_GMin, &QSlider::valueChanged, this, &ThresholdEditor::changed);

            QLabel *number = new QLabel(QStringLiteral("0"));
            number->setMinimumWidth(number->fontMetrics().width(QStringLiteral("00000")));
            h_layout->addWidget(number);
            connect(m_GMin, &QSlider::valueChanged, number, static_cast<void (QLabel::*)(int)>(&QLabel::setNum));

            v_layout->addRow(tr("Grayscale Min"), temp);
        }

        {
            QWidget *temp = new QWidget();
            QHBoxLayout *h_layout = new QHBoxLayout(temp);
            h_layout->setMargin(0);

            m_GMax = new QSlider(Qt::Horizontal);
            m_GMax->setTickInterval(10);
            m_GMax->setTickPosition(QSlider::TicksBelow);
            m_GMax->setSingleStep(1);
            m_GMax->setPageStep(10);
            m_GMax->setRange(0, 255);
            m_GMax->setValue(255);
            h_layout->addWidget(m_GMax);
            connect(m_GMax, &QSlider::valueChanged, this, &ThresholdEditor::changed);

            QLabel *number = new QLabel(QStringLiteral("255"));
            number->setMinimumWidth(number->fontMetrics().width(QStringLiteral("00000")));
            h_layout->addWidget(number);
            connect(m_GMax, &QSlider::valueChanged, number, static_cast<void (QLabel::*)(int)>(&QLabel::setNum));

            v_layout->addRow(tr("Grayscale Max"), temp);
        }

        m_GOut = new QLineEdit(QStringLiteral("(0, 255)"));
        m_GOut->setReadOnly(true);
        v_layout->addRow(tr("Grayscale Threshold"), m_GOut);

        layout->addWidget(m_paneG);
        if(m_combo->currentIndex()) m_paneG->hide();
    }

    // LAB //
    {
        m_paneLAB = new QWidget();
        QFormLayout *v_layout = new QFormLayout(m_paneLAB);
        v_layout->setMargin(0);

        {
            QWidget *temp = new QWidget();
            QHBoxLayout *h_layout = new QHBoxLayout(temp);
            h_layout->setMargin(0);

            m_LMin = new QSlider(Qt::Horizontal);
            m_LMin->setTickInterval(10);
            m_LMin->setTickPosition(QSlider::TicksBelow);
            m_LMin->setSingleStep(1);
            m_LMin->setPageStep(10);
            m_LMin->setRange(0, 100);
            m_LMin->setValue(0);
            h_layout->addWidget(m_LMin);
            connect(m_LMin, &QSlider::valueChanged, this, &ThresholdEditor::changed);

            QLabel *number = new QLabel(QStringLiteral("0"));
            number->setMinimumWidth(number->fontMetrics().width(QStringLiteral("00000")));
            h_layout->addWidget(number);
            connect(m_LMin, &QSlider::valueChanged, number, static_cast<void (QLabel::*)(int)>(&QLabel::setNum));

            v_layout->addRow(tr("L Min"), temp);
        }

        {
            QWidget *temp = new QWidget();
            QHBoxLayout *h_layout = new QHBoxLayout(temp);
            h_layout->setMargin(0);

            m_LMax = new QSlider(Qt::Horizontal);
            m_LMax->setTickInterval(10);
            m_LMax->setTickPosition(QSlider::TicksBelow);
            m_LMax->setSingleStep(1);
            m_LMax->setPageStep(10);
            m_LMax->setRange(0, 100);
            m_LMax->setValue(100);
            h_layout->addWidget(m_LMax);
            connect(m_LMax, &QSlider::valueChanged, this, &ThresholdEditor::changed);

            QLabel *number = new QLabel(QStringLiteral("100"));
            number->setMinimumWidth(number->fontMetrics().width(QStringLiteral("00000")));
            h_layout->addWidget(number);
            connect(m_LMax, &QSlider::valueChanged, number, static_cast<void (QLabel::*)(int)>(&QLabel::setNum));

            v_layout->addRow(tr("L Max"), temp);
        }

        {
            QWidget *temp = new QWidget();
            QHBoxLayout *h_layout = new QHBoxLayout(temp);
            h_layout->setMargin(0);

            m_AMin = new QSlider(Qt::Horizontal);
            m_AMin->setTickInterval(10);
            m_AMin->setTickPosition(QSlider::TicksBelow);
            m_AMin->setSingleStep(1);
            m_AMin->setPageStep(10);
            m_AMin->setRange(-128, 127);
            m_AMin->setValue(-128);
            h_layout->addWidget(m_AMin);
            connect(m_AMin, &QSlider::valueChanged, this, &ThresholdEditor::changed);

            QLabel *number = new QLabel(QStringLiteral("-128"));
            number->setMinimumWidth(number->fontMetrics().width(QStringLiteral("00000")));
            h_layout->addWidget(number);
            connect(m_AMin, &QSlider::valueChanged, number, static_cast<void (QLabel::*)(int)>(&QLabel::setNum));

            v_layout->addRow(tr("A Min"), temp);
        }

        {
            QWidget *temp = new QWidget();
            QHBoxLayout *h_layout = new QHBoxLayout(temp);
            h_layout->setMargin(0);

            m_AMax = new QSlider(Qt::Horizontal);
            m_AMax->setTickInterval(10);
            m_AMax->setTickPosition(QSlider::TicksBelow);
            m_AMax->setSingleStep(1);
            m_AMax->setPageStep(10);
            m_AMax->setRange(-128, 127);
            m_AMax->setValue(127);
            h_layout->addWidget(m_AMax);
            connect(m_AMax, &QSlider::valueChanged, this, &ThresholdEditor::changed);

            QLabel *number = new QLabel(QStringLiteral("127"));
            number->setMinimumWidth(number->fontMetrics().width(QStringLiteral("00000")));
            h_layout->addWidget(number);
            connect(m_AMax, &QSlider::valueChanged, number, static_cast<void (QLabel::*)(int)>(&QLabel::setNum));

            v_layout->addRow(tr("A Max"), temp);
        }

        {
            QWidget *temp = new QWidget();
            QHBoxLayout *h_layout = new QHBoxLayout(temp);
            h_layout->setMargin(0);

            m_BMin = new QSlider(Qt::Horizontal);
            m_BMin->setTickInterval(10);
            m_BMin->setTickPosition(QSlider::TicksBelow);
            m_BMin->setSingleStep(1);
            m_BMin->setPageStep(10);
            m_BMin->setRange(-128, 127);
            m_BMin->setValue(-128);
            h_layout->addWidget(m_BMin);
            connect(m_BMin, &QSlider::valueChanged, this, &ThresholdEditor::changed);

            QLabel *number = new QLabel(QStringLiteral("-128"));
            number->setMinimumWidth(number->fontMetrics().width(QStringLiteral("00000")));
            h_layout->addWidget(number);
            connect(m_BMin, &QSlider::valueChanged, number, static_cast<void (QLabel::*)(int)>(&QLabel::setNum));

            v_layout->addRow(tr("B Min"), temp);
        }

        {
            QWidget *temp = new QWidget();
            QHBoxLayout *h_layout = new QHBoxLayout(temp);
            h_layout->setMargin(0);

            m_BMax = new QSlider(Qt::Horizontal);
            m_BMax->setTickInterval(10);
            m_BMax->setTickPosition(QSlider::TicksBelow);
            m_BMax->setSingleStep(1);
            m_BMax->setPageStep(10);
            m_BMax->setRange(-128, 127);
            m_BMax->setValue(127);
            h_layout->addWidget(m_BMax);
            connect(m_BMax, &QSlider::valueChanged, this, &ThresholdEditor::changed);

            QLabel *number = new QLabel(QStringLiteral("127"));
            number->setMinimumWidth(number->fontMetrics().width(QStringLiteral("00000")));
            h_layout->addWidget(number);
            connect(m_BMax, &QSlider::valueChanged, number, static_cast<void (QLabel::*)(int)>(&QLabel::setNum));

            v_layout->addRow(tr("B Max"), temp);
        }

        m_LABOut = new QLineEdit(QStringLiteral("(0, 100, -128, 127, -128, 127)"));
        m_LABOut->setReadOnly(true);
        v_layout->addRow(tr("LAB Threshold"), m_LABOut);

        layout->addWidget(m_paneLAB);
        if(!m_combo->currentIndex()) m_paneLAB->hide();
    }

    // HSV //
    {
        m_paneHSV = new QWidget();
        QFormLayout *v_layout = new QFormLayout(m_paneHSV);
        v_layout->setMargin(0);

        {
            QWidget *temp = new QWidget();
            QHBoxLayout *h_layout = new QHBoxLayout(temp);
            h_layout->setMargin(0);

            m_HMin = new QSlider(Qt::Horizontal);
            m_HMin->setTickInterval(10);
            m_HMin->setTickPosition(QSlider::TicksBelow);
            m_HMin->setSingleStep(1);
            m_HMin->setPageStep(10);
            m_HMin->setRange(0, 180);
            m_HMin->setValue(0);
            h_layout->addWidget(m_HMin);
            connect(m_HMin, &QSlider::valueChanged, this, &ThresholdEditor::changed);

            QLabel *number = new QLabel(QStringLiteral("0"));
            number->setMinimumWidth(number->fontMetrics().width(QStringLiteral("00000")));
            h_layout->addWidget(number);
            connect(m_HMin, &QSlider::valueChanged, number, static_cast<void (QLabel::*)(int)>(&QLabel::setNum));

            v_layout->addRow(tr("H Min"), temp);
        }

        {
            QWidget *temp = new QWidget();
            QHBoxLayout *h_layout = new QHBoxLayout(temp);
            h_layout->setMargin(0);

            m_HMax = new QSlider(Qt::Horizontal);
            m_HMax->setTickInterval(10);
            m_HMax->setTickPosition(QSlider::TicksBelow);
            m_HMax->setSingleStep(1);
            m_HMax->setPageStep(10);
            m_HMax->setRange(0, 180);
            m_HMax->setValue(180);
            h_layout->addWidget(m_HMax);
            connect(m_HMax, &QSlider::valueChanged, this, &ThresholdEditor::changed);

            QLabel *number = new QLabel(QStringLiteral("180"));
            number->setMinimumWidth(number->fontMetrics().width(QStringLiteral("00000")));
            h_layout->addWidget(number);
            connect(m_HMax, &QSlider::valueChanged, number, static_cast<void (QLabel::*)(int)>(&QLabel::setNum));

            v_layout->addRow(tr("H Max"), temp);
        }

        {
            QWidget *temp = new QWidget();
            QHBoxLayout *h_layout = new QHBoxLayout(temp);
            h_layout->setMargin(0);

            m_SMin = new QSlider(Qt::Horizontal);
            m_SMin->setTickInterval(10);
            m_SMin->setTickPosition(QSlider::TicksBelow);
            m_SMin->setSingleStep(1);
            m_SMin->setPageStep(10);
            m_SMin->setRange(0, 255);
            m_SMin->setValue(0);
            h_layout->addWidget(m_SMin);
            connect(m_SMin, &QSlider::valueChanged, this, &ThresholdEditor::changed);

            QLabel *number = new QLabel(QStringLiteral("0"));
            number->setMinimumWidth(number->fontMetrics().width(QStringLiteral("00000")));
            h_layout->addWidget(number);
            connect(m_SMin, &QSlider::valueChanged, number, static_cast<void (QLabel::*)(int)>(&QLabel::setNum));

            v_layout->addRow(tr("S Min"), temp);
        }

        {
            QWidget *temp = new QWidget();
            QHBoxLayout *h_layout = new QHBoxLayout(temp);
            h_layout->setMargin(0);

            m_SMax = new QSlider(Qt::Horizontal);
            m_SMax->setTickInterval(10);
            m_SMax->setTickPosition(QSlider::TicksBelow);
            m_SMax->setSingleStep(1);
            m_SMax->setPageStep(10);
            m_SMax->setRange(0, 255);
            m_SMax->setValue(255);
            h_layout->addWidget(m_SMax);
            connect(m_SMax, &QSlider::valueChanged, this, &ThresholdEditor::changed);

            QLabel *number = new QLabel(QStringLiteral("255"));
            number->setMinimumWidth(number->fontMetrics().width(QStringLiteral("00000")));
            h_layout->addWidget(number);
            connect(m_SMax, &QSlider::valueChanged, number, static_cast<void (QLabel::*)(int)>(&QLabel::setNum));

            v_layout->addRow(tr("S Max"), temp);
        }

        {
            QWidget *temp = new QWidget();
            QHBoxLayout *h_layout = new QHBoxLayout(temp);
            h_layout->setMargin(0);

            m_VMin = new QSlider(Qt::Horizontal);
            m_VMin->setTickInterval(10);
            m_VMin->setTickPosition(QSlider::TicksBelow);
            m_VMin->setSingleStep(1);
            m_VMin->setPageStep(10);
            m_VMin->setRange(0, 255);
            m_VMin->setValue(0);
            h_layout->addWidget(m_VMin);
            connect(m_VMin, &QSlider::valueChanged, this, &ThresholdEditor::changed);

            QLabel *number = new QLabel(QStringLiteral("0"));
            number->setMinimumWidth(number->fontMetrics().width(QStringLiteral("00000")));
            h_layout->addWidget(number);
            connect(m_VMin, &QSlider::valueChanged, number, static_cast<void (QLabel::*)(int)>(&QLabel::setNum));

            v_layout->addRow(tr("V Min"), temp);
        }

        {
            QWidget *temp = new QWidget();
            QHBoxLayout *h_layout = new QHBoxLayout(temp);
            h_layout->setMargin(0);

            m_VMax = new QSlider(Qt::Horizontal);
            m_VMax->setTickInterval(10);
            m_VMax->setTickPosition(QSlider::TicksBelow);
            m_VMax->setSingleStep(1);
            m_VMax->setPageStep(10);
            m_VMax->setRange(0, 255);
            m_VMax->setValue(255);
            h_layout->addWidget(m_VMax);
            connect(m_VMax, &QSlider::valueChanged, this, &ThresholdEditor::changed);

            QLabel *number = new QLabel(QStringLiteral("255"));
            number->setMinimumWidth(number->fontMetrics().width(QStringLiteral("00000")));
            h_layout->addWidget(number);
            connect(m_VMax, &QSlider::valueChanged, number, static_cast<void (QLabel::*)(int)>(&QLabel::setNum));

            v_layout->addRow(tr("V Max"), temp);
        }

        m_HSVOut = new QLineEdit(QStringLiteral("(0, 180, 0, 255, 0, 255)"));
        m_HSVOut->setReadOnly(true);
        v_layout->addRow(tr("HSV Threshold"), m_HSVOut);

        layout->addWidget(m_paneHSV);
        if(!m_combo->currentIndex()) m_paneHSV->hide();

    }


}

ThresholdEditor::~ThresholdEditor()
{
    delete ui;
}

void ThresholdEditor::resizeEvent(QResizeEvent *event)
{
    //    ui->raw_label->setPixmap(QPixmap::fromImage(m_image.scaled(ui->raw_label->size(), Qt::KeepAspectRatio)));
    //    ui->bin_label->setPixmap(QPixmap::fromImage(m_image.scaled(ui->bin_label->size(), Qt::KeepAspectRatio)));

    QWidget::resizeEvent(event);
}

void ThresholdEditor::showEvent(QShowEvent *event)
{
    restoreGeometry(m_geometry);

    QWidget::showEvent(event);
}

void ThresholdEditor::dragEnterEvent(QDragEnterEvent *event)
{
    event->acceptProposedAction();
}

void ThresholdEditor::dropEvent(QDropEvent *event)
{
#if defined(Q_OS_WIN32)
    QString fileName = event->mimeData()->urls().first().toString().mid(8);
#elif defined(Q_OS_MACOS)
    QString fileName = event->mimeData()->urls().first().toString().mid(7);
#endif
    qDebug() << fileName;
    QMimeType mime = QMimeDatabase().mimeTypeForFile(fileName);
    if (!mime.name().startsWith("image/")) {
        qDebug() << "1111";
        return;
    }
    QPixmap pixmap(fileName);
    m_image = pixmap.scaled(ui->raw_label->size(), Qt::KeepAspectRatio).toImage();
    ui->raw_label->setPixmap(QPixmap::fromImage(m_image));

    changed();
}


void ThresholdEditor::changed()
{
    clock_t start,finish;
    double totaltime;
    start=clock();
    static QRgb pixel;

    QImage out(m_image.width(), m_image.height(), QImage::Format_Grayscale8);

    switch (m_combo->currentIndex())
    {
    case 0:
    {
        m_paneG->show();
        m_paneLAB->hide();
        m_paneHSV->hide();

        for(int y = 0; y < m_image.height(); y++)
        {
            for(int x = 0; x < m_image.width(); x++)
            {
                pixel = m_image.pixel(x, y);
                bool GMinOk = toGrayscale(pixel) >= qMin(m_GMin->value(), m_GMax->value());
                bool GMaxOk = toGrayscale(pixel) <= qMax(m_GMax->value(), m_GMin->value());
                bool allOk = (GMinOk && GMaxOk) ^ m_invert->isChecked();
                out.setPixel(x, y, allOk ? -1 : 0);
            }
        }

        m_GOut->setText(QStringLiteral("(%L1, %L2)").arg(m_GMin->value())
                        .arg(m_GMax->value()));
    }break;
    case 1:
    {
        m_paneG->hide();
        m_paneLAB->show();
        m_paneHSV->hide();

        for(int y = 0; y < m_image.height(); y++)
        {
            for(int x = 0; x < m_image.width(); x++)
            {
                pixel = m_image.pixel(x, y);
                bool LMinOk = toL(pixel) >= qMin(m_LMin->value(), m_LMax->value());
                bool LMaxOk = toL(pixel) <= qMax(m_LMax->value(), m_LMin->value());
                bool AMinOk = toA(pixel) >= qMin(m_AMin->value(), m_AMax->value());
                bool AMaxOk = toA(pixel) <= qMax(m_AMax->value(), m_AMin->value());
                bool BMinOk = toB(pixel) >= qMin(m_BMin->value(), m_BMax->value());
                bool BMaxOk = toB(pixel) <= qMax(m_BMax->value(), m_BMin->value());
                bool allOk = (LMinOk && LMaxOk && AMinOk && AMaxOk && BMinOk && BMaxOk) ^ m_invert->isChecked();
                out.setPixel(x, y, allOk ? -1 : 0);
            }
        }

        m_LABOut->setText(QStringLiteral("(%L1, %L2, %L3, %L4, %L5, %L6)").arg(m_LMin->value())
                          .arg(m_LMax->value())
                          .arg(m_AMin->value())
                          .arg(m_AMax->value())
                          .arg(m_BMin->value())
                          .arg(m_BMax->value()));
    }break;
    case 2:
    {
        m_paneG->hide();
        m_paneLAB->hide();
        m_paneHSV->show();

        for(int y = 0; y < m_image.height(); y++)
        {
            for(int x = 0; x < m_image.width(); x++)
            {
                pixel = m_image.pixel(x, y);
                QColor temp(pixel);
                bool HMinOk = temp.hue()/2 >= qMin(m_HMin->value(), m_HMax->value());
                bool HMaxOk = temp.hue()/2 <= qMax(m_HMax->value(), m_HMin->value());
                bool SMinOk = temp.saturation() >= qMin(m_SMin->value(), m_SMax->value());
                bool SMaxOk = temp.saturation() <= qMax(m_SMax->value(), m_SMin->value());
                bool VMinOk = temp.value() >= qMin(m_VMin->value(), m_VMax->value());
                bool VMaxOk = temp.value() <= qMax(m_VMax->value(), m_VMin->value());
                bool allOk = (HMinOk && HMaxOk && SMinOk && SMaxOk && VMinOk && VMaxOk) ^ m_invert->isChecked();
                out.setPixel(x, y, allOk ? -1 : 0);
            }
        }

        m_HSVOut->setText(QStringLiteral("(%L1, %L2, %L3, %L4, %L5, %L6)").arg(m_HMin->value())
                          .arg(m_HMax->value())
                          .arg(m_SMin->value())
                          .arg(m_SMax->value())
                          .arg(m_VMin->value())
                          .arg(m_VMax->value()));
    }break;
    }
    ui->bin_label->setPixmap(QPixmap::fromImage(out));

    finish=clock();
    totaltime=(double)(finish-start)/CLOCKS_PER_SEC;
    qDebug()<<"\n此程序的运行时间为"<<totaltime<<"秒！"<<endl;


    if(sender() == m_combo)
    {
        QSize s = size();
        adjustSize();
        resize(s);
    }
}


#include "switchbutton.h"
#include <QPainter>
#include <QMouseEvent>
#include <QPropertyAnimation>
#include <QLabel>

SwitchButton::SwitchButton(QWidget *parent) : QWidget(parent), m_checked(true), m_sliderPos(1) {
    setFixedSize(105, 40);
    initLabel();
    connect(this, &SwitchButton::sliderPosChanged, this, [=](int newPos) {
        m_sliderPos = newPos;
        update();
    });

}

void SwitchButton::setChecked(bool checked) {
    if (m_checked != checked) {
        m_checked = checked;

        int sliderRadius = height() / 2 - 2;
        // Рассчитываем начальную и конечную позицию анимации
        int startValue = m_checked ? width() - 45 - sliderRadius * 2 - 1 : 1;
        int endValue = m_checked ? 1 : width() - 45 - sliderRadius * 2 - 1;

        // Создаем анимацию
        QPropertyAnimation *animation = new QPropertyAnimation(this, "sliderPos");
        animation->setDuration(200); // Продолжительность анимации в миллисекундах
        animation->setStartValue(startValue);
        animation->setEndValue(endValue);
        animation->start();

        // При завершении анимации отправляем сигнал toggled
        connect(animation, &QPropertyAnimation::finished, this, [=]() {
            emit toggled(m_checked);
        });

    }
    updateLabel();
}

void SwitchButton::resizeEvent(QResizeEvent *event) {
    Q_UNUSED(event);
    int newHeight = 44; // Новая высота виджета
    setFixedHeight(newHeight);
}

void SwitchButton::initLabel() {
    m_label = new QLabel(this);
    m_label->setStyleSheet("QLabel { padding-left: 2px; margin: 15px 0 0 55px; background-color: transparent; }");
    m_label->setAlignment(Qt::AlignCenter);
    m_label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    updateLabel(); // Установка исходного текста метки
}


void SwitchButton::updateLabel() {
    if (m_checked) {
        m_label->setText(" Include");
    } else {
        m_label->setText(" Exclude");
    }
}

bool SwitchButton::isChecked() const {
    return m_checked;
}

void SwitchButton::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        setChecked(!m_checked);
        event->accept();
    } else {
        event->ignore();
    }
}

void SwitchButton::paintEvent(QPaintEvent *event) {
    int m_fixedMargin = 10; // Фиксированный отступ в 10 пикселей

    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QColor bgColor = m_checked ? QColor(76, 175, 80) : QColor(221, 0, 0);
    QColor sliderColor = QColor(255, 255, 255);

    painter.setPen(Qt::NoPen);

    // Draw background
    QRect backgroundRect = rect().adjusted(m_fixedMargin, m_fixedMargin, -55, -m_fixedMargin);
    painter.setBrush(bgColor);
    painter.drawRoundedRect(backgroundRect, backgroundRect.height() / 2, backgroundRect.height() / 2);

    // Draw slider
    int sliderRadius = (height() - 2 * m_fixedMargin) / 2 - 2;
    int sliderPosX = m_sliderPos + m_fixedMargin;
    painter.setBrush(sliderColor);
    painter.drawEllipse(sliderPosX, m_fixedMargin + 2, sliderRadius * 2, sliderRadius * 2);
    updateLabel();
}


void SwitchButton::setSliderPos(int pos) {
    if (m_sliderPos != pos) {
        m_sliderPos = pos;
        emit sliderPosChanged(m_sliderPos);
        update(); // Вызов перерисовки, если необходимо
    }
}



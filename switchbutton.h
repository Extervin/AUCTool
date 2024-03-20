#ifndef SWITCHBUTTON_H
#define SWITCHBUTTON_H

#include <QWidget>
#include <QLabel>

class SwitchButton : public QWidget {
    Q_OBJECT
    Q_PROPERTY(int sliderPos READ sliderPos WRITE setSliderPos NOTIFY sliderPosChanged)

public:
    explicit SwitchButton(QWidget *parent = nullptr);

    void setChecked(bool checked);
    bool isChecked() const;

    int sliderPos() const { return m_sliderPos; }
    void setSliderPos(int pos);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

signals:
    void toggled(bool checked);
    void sliderPosChanged(int pos);

private:
    bool m_checked;
    int m_sliderPos;
    void initLabel();
    void updateLabel();
    void resizeEvent(QResizeEvent *event);

    QLabel *m_label;
};



#endif // SWITCHBUTTON_H

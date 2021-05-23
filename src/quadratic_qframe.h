#pragma once

#include <QFrame>
#include <QResizeEvent>

class QuadraticQFrame : public QFrame {
    public:
        QuadraticQFrame(QWidget *parent) : QFrame(parent) {}
        ~QuadraticQFrame() noexcept {}

        auto resizeEvent(QResizeEvent *event) -> void {
            event->accept();

            if(event->size().width() > event->size().height()){
                this->resize(event->size().height(),event->size().height());
            }else{
                this->resize(event->size().width(),event->size().width());
            }
        }
};

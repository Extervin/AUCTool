#include "confirmation.h"
#include "ui_confirmation.h"

Confirmation::Confirmation(QWidget *parent)
    : QDialog(parent),
    ui(new Ui::Confirmation)
{}

Confirmation::~Confirmation()
{
    delete ui;
}

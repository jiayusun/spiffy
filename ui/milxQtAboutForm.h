#ifndef MILXQTABOUTFORM_H
#define MILXQTABOUTFORM_H

#include "ui_about.h"
#include "milxQtAliases.h"
/*!
    \class milxQtAboutForm
    \brief This class represents the about form and other info.

    It has SPIFFY Logo, author and build libraries and other info.
*/
class MILXQT_EXPORT milxQtAboutForm : public QDialog
{
    Q_OBJECT

public:
    milxQtAboutForm(QWidget *theParent = 0);
    virtual ~milxQtAboutForm();

    void setupVersion();

protected:
    Ui_dlgAbout ui;

    void createConnections();
};

#endif // MILXQTABOUTFORM_H

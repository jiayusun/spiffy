#include "milxQtAboutForm.h"
#include "milxImage.h"

milxQtAboutForm::milxQtAboutForm(QWidget *theParent) : QDialog(theParent)
{
    ui.setupUi(this);

    setWindowModality(Qt::ApplicationModal); //block user input

    setupVersion();
    createConnections();
}

milxQtAboutForm::~milxQtAboutForm()
{
    //dtor
}

void milxQtAboutForm::setupVersion()
{
    QString itkStr = QString::number(ITK_VERSION_MAJOR) + "." + QString::number(ITK_VERSION_MINOR) + "." + QString::number(ITK_VERSION_PATCH);
    QString vtkStr = QString::number(VTK_MAJOR_VERSION) + "." + QString::number(VTK_MINOR_VERSION) + "." + QString::number(VTK_BUILD_VERSION);

    ui.aboutEdit->insertPlainText("ITK Version: " + itkStr + "\n");
    ui.aboutEdit->insertPlainText("VTK Version: " + vtkStr + "\n");
    ui.aboutEdit->insertPlainText("Qt Version: " + QString(QT_VERSION_STR) + "\n");
    ui.aboutEdit->setReadOnly(true);
}

void milxQtAboutForm::createConnections()
{

}

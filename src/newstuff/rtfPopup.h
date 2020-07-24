using namespace cli;

#include <QMainWindow>

class RtfPopup : public QMainWindow
{
  Q_OBJECT

public:
  RtfPopup(System::String^ rtfText, QWidget* parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());
};

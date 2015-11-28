#include "receiver.h"
#include <QWidget>
using namespace std;

int main(int argc, char *argv[]){
	receiver_t receiver;
	receiver_initialize(&receiver, RECV_AUDIOCARD);
	receiver_start(&receiver);

	receiver.freq_displayer->d_qApplication = new QApplication(argc, argv);
	receiver.freq_displayer->qwidget()->show();
	receiver.freq_displayer->exec_();
}
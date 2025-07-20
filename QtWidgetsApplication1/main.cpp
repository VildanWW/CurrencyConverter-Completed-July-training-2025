#include <QtWidgets/QApplication>
#include <string>
#include <QtCharts/qchartview.h>
#include "MainMenu.h"
#include <curl/curl.h>

int main(int argc, char* argv[]) {
    curl_global_init(CURL_GLOBAL_DEFAULT);   // <-- �����
    QApplication app(argc, argv);
    QtWidgetsApplication1 window;
    window.show();
    int rc = app.exec();
    curl_global_cleanup();                   // <-- �� ����������
    return rc;
}


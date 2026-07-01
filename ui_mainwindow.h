/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 5.12.12
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSlider>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QSplitter>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
#include "spectrumplotwidget.h"
#include "waterfallwidget.h"

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralWidget;
    QHBoxLayout *rootLayout;
    QSplitter *mainSplitter;
    QWidget *leftPanel;
    QVBoxLayout *leftLayout;
    QGroupBox *controlBox;
    QGridLayout *controlLayout;
    QLabel *label_scenario;
    QComboBox *scenarioCombo;
    QLabel *label_threshold;
    QSlider *thresholdSlider;
    QLabel *thresholdLabel;
    QPushButton *runButton;
    QPushButton *clearButton;
    QGroupBox *resultBox;
    QVBoxLayout *resultLayout;
    QLabel *statusLabel;
    QLabel *peakLabel;
    QLabel *bandwidthLabel;
    QLabel *confidenceLabel;
    QSpacerItem *verticalSpacer;
    QLabel *label_events;
    QTableWidget *eventTable;
    QSplitter *rightSplitter;
    QWidget *freqControlPanel;
    QHBoxLayout *freqControlLayout;
    QHBoxLayout *horizontalLayout;
    QLabel *label_startFreq;
    QDoubleSpinBox *startFreqSpin;
    QHBoxLayout *horizontalLayout_3;
    QLabel *label_centerFreq;
    QDoubleSpinBox *centerFreqSpin;
    QHBoxLayout *horizontalLayout_2;
    QLabel *label_span;
    QDoubleSpinBox *spanSpin;
    QSpacerItem *freqControlSpacer;
    QPushButton *applyFreqButton;
    SpectrumPlotWidget *spectrumPlotWidget;
    WaterfallWidget *waterfallWidget;
    QMenuBar *menuBar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QString::fromUtf8("MainWindow"));
        MainWindow->resize(1280, 760);
        centralWidget = new QWidget(MainWindow);
        centralWidget->setObjectName(QString::fromUtf8("centralWidget"));
        rootLayout = new QHBoxLayout(centralWidget);
        rootLayout->setSpacing(0);
        rootLayout->setObjectName(QString::fromUtf8("rootLayout"));
        rootLayout->setContentsMargins(10, 10, 10, 10);
        mainSplitter = new QSplitter(centralWidget);
        mainSplitter->setObjectName(QString::fromUtf8("mainSplitter"));
        mainSplitter->setOrientation(Qt::Horizontal);
        mainSplitter->setChildrenCollapsible(false);
        leftPanel = new QWidget(mainSplitter);
        leftPanel->setObjectName(QString::fromUtf8("leftPanel"));
        leftLayout = new QVBoxLayout(leftPanel);
        leftLayout->setSpacing(10);
        leftLayout->setObjectName(QString::fromUtf8("leftLayout"));
        leftLayout->setContentsMargins(0, 0, 0, 0);
        controlBox = new QGroupBox(leftPanel);
        controlBox->setObjectName(QString::fromUtf8("controlBox"));
        controlLayout = new QGridLayout(controlBox);
        controlLayout->setSpacing(6);
        controlLayout->setObjectName(QString::fromUtf8("controlLayout"));
        label_scenario = new QLabel(controlBox);
        label_scenario->setObjectName(QString::fromUtf8("label_scenario"));

        controlLayout->addWidget(label_scenario, 0, 0, 1, 1);

        scenarioCombo = new QComboBox(controlBox);
        scenarioCombo->setObjectName(QString::fromUtf8("scenarioCombo"));

        controlLayout->addWidget(scenarioCombo, 0, 1, 1, 2);

        label_threshold = new QLabel(controlBox);
        label_threshold->setObjectName(QString::fromUtf8("label_threshold"));

        controlLayout->addWidget(label_threshold, 1, 0, 1, 1);

        thresholdSlider = new QSlider(controlBox);
        thresholdSlider->setObjectName(QString::fromUtf8("thresholdSlider"));
        thresholdSlider->setOrientation(Qt::Horizontal);

        controlLayout->addWidget(thresholdSlider, 1, 1, 1, 1);

        thresholdLabel = new QLabel(controlBox);
        thresholdLabel->setObjectName(QString::fromUtf8("thresholdLabel"));

        controlLayout->addWidget(thresholdLabel, 1, 2, 1, 1);

        runButton = new QPushButton(controlBox);
        runButton->setObjectName(QString::fromUtf8("runButton"));

        controlLayout->addWidget(runButton, 2, 1, 1, 1);

        clearButton = new QPushButton(controlBox);
        clearButton->setObjectName(QString::fromUtf8("clearButton"));

        controlLayout->addWidget(clearButton, 2, 2, 1, 1);


        leftLayout->addWidget(controlBox);

        resultBox = new QGroupBox(leftPanel);
        resultBox->setObjectName(QString::fromUtf8("resultBox"));
        resultLayout = new QVBoxLayout(resultBox);
        resultLayout->setSpacing(6);
        resultLayout->setObjectName(QString::fromUtf8("resultLayout"));
        statusLabel = new QLabel(resultBox);
        statusLabel->setObjectName(QString::fromUtf8("statusLabel"));
        statusLabel->setWordWrap(true);

        resultLayout->addWidget(statusLabel);

        peakLabel = new QLabel(resultBox);
        peakLabel->setObjectName(QString::fromUtf8("peakLabel"));

        resultLayout->addWidget(peakLabel);

        bandwidthLabel = new QLabel(resultBox);
        bandwidthLabel->setObjectName(QString::fromUtf8("bandwidthLabel"));

        resultLayout->addWidget(bandwidthLabel);

        confidenceLabel = new QLabel(resultBox);
        confidenceLabel->setObjectName(QString::fromUtf8("confidenceLabel"));

        resultLayout->addWidget(confidenceLabel);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        resultLayout->addItem(verticalSpacer);


        leftLayout->addWidget(resultBox);

        label_events = new QLabel(leftPanel);
        label_events->setObjectName(QString::fromUtf8("label_events"));

        leftLayout->addWidget(label_events);

        eventTable = new QTableWidget(leftPanel);
        if (eventTable->columnCount() < 5)
            eventTable->setColumnCount(5);
        QTableWidgetItem *__qtablewidgetitem = new QTableWidgetItem();
        eventTable->setHorizontalHeaderItem(0, __qtablewidgetitem);
        QTableWidgetItem *__qtablewidgetitem1 = new QTableWidgetItem();
        eventTable->setHorizontalHeaderItem(1, __qtablewidgetitem1);
        QTableWidgetItem *__qtablewidgetitem2 = new QTableWidgetItem();
        eventTable->setHorizontalHeaderItem(2, __qtablewidgetitem2);
        QTableWidgetItem *__qtablewidgetitem3 = new QTableWidgetItem();
        eventTable->setHorizontalHeaderItem(3, __qtablewidgetitem3);
        QTableWidgetItem *__qtablewidgetitem4 = new QTableWidgetItem();
        eventTable->setHorizontalHeaderItem(4, __qtablewidgetitem4);
        eventTable->setObjectName(QString::fromUtf8("eventTable"));
        eventTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
        eventTable->setSelectionBehavior(QAbstractItemView::SelectRows);
        eventTable->setColumnCount(5);
        eventTable->horizontalHeader()->setVisible(true);
        eventTable->verticalHeader()->setVisible(false);

        leftLayout->addWidget(eventTable);

        mainSplitter->addWidget(leftPanel);
        rightSplitter = new QSplitter(mainSplitter);
        rightSplitter->setObjectName(QString::fromUtf8("rightSplitter"));
        rightSplitter->setOrientation(Qt::Vertical);
        rightSplitter->setChildrenCollapsible(false);
        freqControlPanel = new QWidget(rightSplitter);
        freqControlPanel->setObjectName(QString::fromUtf8("freqControlPanel"));
        freqControlLayout = new QHBoxLayout(freqControlPanel);
        freqControlLayout->setSpacing(8);
        freqControlLayout->setObjectName(QString::fromUtf8("freqControlLayout"));
        freqControlLayout->setContentsMargins(10, 8, 10, 8);
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setSpacing(2);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        label_startFreq = new QLabel(freqControlPanel);
        label_startFreq->setObjectName(QString::fromUtf8("label_startFreq"));

        horizontalLayout->addWidget(label_startFreq);

        startFreqSpin = new QDoubleSpinBox(freqControlPanel);
        startFreqSpin->setObjectName(QString::fromUtf8("startFreqSpin"));
        startFreqSpin->setDecimals(1);
        startFreqSpin->setMinimum(500.000000000000000);
        startFreqSpin->setMaximum(5500.000000000000000);
        startFreqSpin->setSingleStep(10.000000000000000);
        startFreqSpin->setValue(2350.000000000000000);

        horizontalLayout->addWidget(startFreqSpin);


        freqControlLayout->addLayout(horizontalLayout);

        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setSpacing(2);
        horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
        label_centerFreq = new QLabel(freqControlPanel);
        label_centerFreq->setObjectName(QString::fromUtf8("label_centerFreq"));

        horizontalLayout_3->addWidget(label_centerFreq);

        centerFreqSpin = new QDoubleSpinBox(freqControlPanel);
        centerFreqSpin->setObjectName(QString::fromUtf8("centerFreqSpin"));
        centerFreqSpin->setDecimals(1);
        centerFreqSpin->setMinimum(500.000000000000000);
        centerFreqSpin->setMaximum(6000.000000000000000);
        centerFreqSpin->setSingleStep(10.000000000000000);
        centerFreqSpin->setValue(2400.000000000000000);

        horizontalLayout_3->addWidget(centerFreqSpin);


        freqControlLayout->addLayout(horizontalLayout_3);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setSpacing(2);
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        label_span = new QLabel(freqControlPanel);
        label_span->setObjectName(QString::fromUtf8("label_span"));

        horizontalLayout_2->addWidget(label_span);

        spanSpin = new QDoubleSpinBox(freqControlPanel);
        spanSpin->setObjectName(QString::fromUtf8("spanSpin"));
        spanSpin->setDecimals(1);
        spanSpin->setMinimum(1.000000000000000);
        spanSpin->setMaximum(5500.000000000000000);
        spanSpin->setSingleStep(10.000000000000000);
        spanSpin->setValue(100.000000000000000);

        horizontalLayout_2->addWidget(spanSpin);


        freqControlLayout->addLayout(horizontalLayout_2);

        freqControlSpacer = new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        freqControlLayout->addItem(freqControlSpacer);

        applyFreqButton = new QPushButton(freqControlPanel);
        applyFreqButton->setObjectName(QString::fromUtf8("applyFreqButton"));

        freqControlLayout->addWidget(applyFreqButton);

        rightSplitter->addWidget(freqControlPanel);
        spectrumPlotWidget = new SpectrumPlotWidget(rightSplitter);
        spectrumPlotWidget->setObjectName(QString::fromUtf8("spectrumPlotWidget"));
        rightSplitter->addWidget(spectrumPlotWidget);
        waterfallWidget = new WaterfallWidget(rightSplitter);
        waterfallWidget->setObjectName(QString::fromUtf8("waterfallWidget"));
        rightSplitter->addWidget(waterfallWidget);
        mainSplitter->addWidget(rightSplitter);

        rootLayout->addWidget(mainSplitter);

        MainWindow->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(MainWindow);
        menuBar->setObjectName(QString::fromUtf8("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 1280, 21));
        MainWindow->setMenuBar(menuBar);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QApplication::translate("MainWindow", "\346\227\240\344\272\272\346\234\272\351\242\221\350\260\261\347\200\221\345\270\203\345\233\276\350\257\206\345\210\253\346\243\200\346\265\213\346\250\241\345\235\227 - Qt 5.12.12", nullptr));
        controlBox->setTitle(QApplication::translate("MainWindow", "\346\250\241\346\213\237\344\270\216\346\243\200\346\265\213\346\216\247\345\210\266", nullptr));
        label_scenario->setText(QApplication::translate("MainWindow", "\346\250\241\346\213\237\345\234\272\346\231\257", nullptr));
        label_threshold->setText(QApplication::translate("MainWindow", "\346\243\200\346\265\213\351\227\250\351\231\220", nullptr));
        thresholdLabel->setText(QApplication::translate("MainWindow", "-68 dBm", nullptr));
        runButton->setText(QApplication::translate("MainWindow", "\346\232\202\345\201\234", nullptr));
        clearButton->setText(QApplication::translate("MainWindow", "\346\270\205\347\251\272", nullptr));
        resultBox->setTitle(QApplication::translate("MainWindow", "\350\257\206\345\210\253\347\273\223\346\236\234", nullptr));
        statusLabel->setText(QApplication::translate("MainWindow", "\347\255\211\345\276\205\346\225\260\346\215\256", nullptr));
        peakLabel->setText(QApplication::translate("MainWindow", "\345\263\260\345\200\274\345\212\237\347\216\207: -", nullptr));
        bandwidthLabel->setText(QApplication::translate("MainWindow", "\345\215\240\347\224\250\345\270\246\345\256\275: -", nullptr));
        confidenceLabel->setText(QApplication::translate("MainWindow", "\347\275\256\344\277\241\345\272\246: -", nullptr));
        label_events->setText(QApplication::translate("MainWindow", "\346\243\200\346\265\213\344\272\213\344\273\266", nullptr));
        QTableWidgetItem *___qtablewidgetitem = eventTable->horizontalHeaderItem(0);
        ___qtablewidgetitem->setText(QApplication::translate("MainWindow", "\346\227\266\351\227\264", nullptr));
        QTableWidgetItem *___qtablewidgetitem1 = eventTable->horizontalHeaderItem(1);
        ___qtablewidgetitem1->setText(QApplication::translate("MainWindow", "\347\273\223\346\236\234", nullptr));
        QTableWidgetItem *___qtablewidgetitem2 = eventTable->horizontalHeaderItem(2);
        ___qtablewidgetitem2->setText(QApplication::translate("MainWindow", "\345\263\260\345\200\274dBm", nullptr));
        QTableWidgetItem *___qtablewidgetitem3 = eventTable->horizontalHeaderItem(3);
        ___qtablewidgetitem3->setText(QApplication::translate("MainWindow", "\344\270\255\345\277\203MHz", nullptr));
        QTableWidgetItem *___qtablewidgetitem4 = eventTable->horizontalHeaderItem(4);
        ___qtablewidgetitem4->setText(QApplication::translate("MainWindow", "\345\270\246\345\256\275MHz", nullptr));
        label_startFreq->setText(QApplication::translate("MainWindow", "\350\265\267\345\247\213\351\242\221\347\216\207", nullptr));
        label_centerFreq->setText(QApplication::translate("MainWindow", "\344\270\255\345\277\203\351\242\221\347\216\207", nullptr));
        label_span->setText(QApplication::translate("MainWindow", "\345\270\246\345\256\275", nullptr));
        applyFreqButton->setText(QApplication::translate("MainWindow", "\345\272\224\347\224\250", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H

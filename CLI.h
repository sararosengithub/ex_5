
#ifndef CLI_H_
#define CLI_H_

#include <string.h>
#include "commands.h"
#include "timeseries.h"
#include "AnomalyDetector.h"

using namespace std;
class CLI {
    DefaultIO* dio;
    Data* data;
    vector<Command*> cammands;
    Upload_command* uploadCommand;
    Algorithem_setting_command* algorithemSettingCommand;
    Anomaly_detection_command* anomalyDetectionCommand;
    Display_results_command* displayResultsCommand;
    Analyze_results_command* analyzeResultsCommand;
    Exit_command* exitCommand;
    // you can add data members
public:
    CLI(DefaultIO* dio);
    void start();
    virtual ~CLI();
};

#endif /* CLI_H_ */

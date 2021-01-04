
#ifndef COMMANDS_H_
#define COMMANDS_H_

#include<iostream>
#include <string.h>

#include <fstream>
#include <vector>
#include "HybridAnomalyDetector.h"
#include "sstream"

using namespace std;

class DefaultIO {
public:
    virtual string read() = 0;

    virtual void write(string text) = 0;

    virtual void write(float f) = 0;

    virtual void read(float *f) = 0;

    virtual ~DefaultIO() {}

    // you may add additional methods here
};
class StandardIO: public DefaultIO{
    string StandardIO::read(){
        string string1;
        cin>>string1;
        return string1;
    }

    void StandardIO::write(string text){
        cout<<text<<endl;
    }

    void StandardIO::write(float f){
        cout<<f<<endl;
    }

    void StandardIO::read(float *f){
        cin>>*f;
    }
};
// you may add here helper classes
struct Data{
    TimeSeries* train_ts;
    TimeSeries* test_ts;
    HybridAnomalyDetector detector;
    vector<AnomalyReport> anomalyreports;
    int line_count_test;
};

// you may edit this class
class Command {
protected:
    DefaultIO *dio;
public:
    string description = "defult description";
    struct Data* data;

    Command(DefaultIO *dio, Data *data) : dio(dio), data(data) {
    }

    virtual void execute() = 0;

    virtual ~Command() {}
};

// implement here your command classes
class Upload_command : public Command {
public:
    Upload_command(DefaultIO *dio, Data *data)
            : Command(dio, data) {
        this->description = "1.upload a time series csv file\n";
    }

    void execute() {
        //upload train csv:
        ofstream trainFile("traindata.csv");
        dio->write("Please upload your local train CSV file.\n");
        string line = "";
        do {
            line = dio->read();
            if (line!="done") trainFile << line << endl;
        } while (line != "done");
        dio->write("Upload complete.\n");
        //TimeSeries traintimeSeries("traindata.csv");
        this->data->train_ts = new TimeSeries("traindata.csv");
        //this->data->train_ts = &traintimeSeries;
        trainFile.close();

        //upload test csv:
        ofstream testFile("testdata.csv");
        dio->write("Please upload your local test CSV file.\n");
        data->line_count_test = 0;
        do {
            line = dio->read();
            if (line!="done"){
            testFile << line << endl;
            data->line_count_test++;}
        } while (line != "done");
        dio->write("Upload complete.\n");
       // TimeSeries testtimeSeries("testdata.csv");
        this->data->test_ts  =new TimeSeries("testdata.csv");
        //data->test_ts = &testtimeSeries;
        testFile.close();

    }
};

class Algorithem_setting_command : public Command {
public:
    Algorithem_setting_command(DefaultIO *dio, Data *data)
            : Command(dio, data) {
        this->description = "2.algorithm settings\n";
    }

    void execute() {
        float correlation_threshold = data->detector.get_correlation_threshold();
        dio->write("The current correlation threshold is " + to_string(correlation_threshold) + '\n' +
                   "Type a new threshold\n");
        correlation_threshold = stof(dio->read());
        data->detector.set_correlation_threshold(correlation_threshold);
    }
};

class Anomaly_detection_command : public Command {
public:
    Anomaly_detection_command(DefaultIO *dio, Data *data)
            : Command(dio, data) {
        this->description = "3.detect anomalies\n";
    }

    void execute() {
        data->detector.learnNormal(*data->train_ts);
        data->anomalyreports = data->detector.detect(*data->test_ts);
        dio->write("anomaly detection complete.\n");
    }
};

class Display_results_command : public Command {
public:
    Display_results_command(DefaultIO *dio, Data *data)
            : Command(dio, data) {
        this->description = "4.display results\n";
    }

    void execute() {
        for (int i = 0; i < data->anomalyreports.size(); i++) {
            dio->write(to_string(data->anomalyreports[i].timeStep) +
                       '\t'+" " + data->anomalyreports[i].description+'\n');
        }
        dio->write("Done.\n");
    }
};

class Analyze_results_command : public Command {
public:
    Analyze_results_command(DefaultIO *dio, Data *data)
            : Command(dio, data) {
        this->description = "5.upload anomalies and analyze results\n";
    }

    void execute() {
        dio->write("Please upload your local anomalies file.\n");
        //create vector that holds start and end time of each anamoly given by user
        vector<pair<long, long>> anomaly_instances_reported;
        int positive_steps = 0;
        string line;
        long time;
        do {
            line = dio->read();
            if(line!="done"){
            stringstream ss(line);
            vector<string> times;
            string intermediate;
            while (getline(ss, intermediate, ',')) {
                times.push_back(intermediate);
            }
            anomaly_instances_reported.push_back(make_pair(stol(times.at(0)),
                                                           stol(times.at(1))));
            positive_steps += stol(times.at(1)) - stol(times.at(0)) + 1;
            times.clear();
            }
        } while (line != "done");
        dio->write("Upload complete.\n");
        //devide anomaly reports into chuncks of time:

        vector<pair<long, long>> anomaly_instances;
        string anamoly_description = data->anomalyreports[0].description;
        long timestart = data->anomalyreports[0].timeStep;
        long timestep = data->anomalyreports[0].timeStep;
        int i = 1;

        // for (int i = 1;i<data->anomalyreports.size();i++){
        do {
            while ((data->anomalyreports[i].description == anamoly_description) &&
                   (data->anomalyreports[i].timeStep == timestep + 1)) {
                timestep++;
                if(i == data->anomalyreports.size()-1){break;}
                i++;
            }
            anomaly_instances.push_back(make_pair(timestart, timestep));
            anamoly_description = data->anomalyreports[i].description;
            timestart = data->anomalyreports[i].timeStep;
            timestep = timestart;
            i++;
        } while (i < data->anomalyreports.size());
        //anomaly_instances.push_back(make_pair(timestart, timestep));

        //calculate p and n:
        float P = anomaly_instances_reported.size();
        float N = data->line_count_test - positive_steps;
        float TP = 0, FP = 0;
        bool flag = 0;

        //iterate over instances in anomaly_instance and check if they overlap with anomaly_instance_reported.
        for (int j = 0; j < anomaly_instances_reported.size(); j++) {
            flag = 0;
            for (int k = 0; k < anomaly_instances.size(); k++) {
                //if left side of a is bigger then right side of b
                if (anomaly_instances.at(k).first > anomaly_instances_reported.at(j).second) {
                    FP++;
                    continue;
                }
                    //if right side of a is smaller then left side of b
                else if (anomaly_instances[k].second < anomaly_instances_reported[j].first) {
                    FP++;
                    continue;
                }
                    //option1 - left side of a is in range b
                else if (anomaly_instances[k].first >= anomaly_instances_reported[j].first &&
                         anomaly_instances[k].first <= anomaly_instances_reported[j].second) {
                    flag = 1;
                    break;
                }//option2 - right side of a is in range b
                else if (anomaly_instances[k].second <= anomaly_instances_reported[j].second &&
                         anomaly_instances[k].second >= anomaly_instances_reported[j].first) {
                    flag = 1;
                    break;
                }//option3 - b is in range a
                else if (anomaly_instances[k].first <= anomaly_instances_reported[j].first &&
                         anomaly_instances[k].second >= anomaly_instances_reported[j].second) {
                    flag = 1;
                    break;
                }
            }
            if (flag) TP++;
        }float true_positive_rate = TP/P;
        //true_positive_rate = (int)(true_positive_rate*1000);
        //true_positive_rate = float(true_positive_rate)/1000;
        float false_alarm_rate = FP/N;
        dio->write("True Positive Rate: "+to_string(true_positive_rate)+'\n');
        dio->write("False Positive Rate: "+to_string(false_alarm_rate)+'\n');

    }
};

class Exit_command : public Command {
public:
    Exit_command(DefaultIO *dio, Data *data)
            : Command(dio, data) {
        this->description = "6.exit\n";
    }
    void execute(){
        delete this->data->train_ts;
        delete this->data->test_ts;
    }
};

#endif /* COMMANDS_H_ */

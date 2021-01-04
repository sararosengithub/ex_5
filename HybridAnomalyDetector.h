

#ifndef HYBRIDANOMALYDETECTOR_H_
#define HYBRIDANOMALYDETECTOR_H_

#include "SimpleAnomalyDetector.h"
#include "minCircle.h"

class HybridAnomalyDetector:public SimpleAnomalyDetector {
    float correlation_threshold;
public:
    HybridAnomalyDetector();
    virtual ~HybridAnomalyDetector();
    //void calculate_linear_regression(const TimeSeries &ts);
    bool is_anomaly(Point *p,correlatedFeatures c) override;
    correlatedFeatures createcorrelatedfeature(Point **p, int size1, correlatedFeatures c)override;
    float get_correlation_threshold();
    void set_correlation_threshold(float f);
};

#endif /* HYBRIDANOMALYDETECTOR_H_ */

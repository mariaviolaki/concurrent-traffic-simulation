#ifndef TRAFFICLIGHT_H
#define TRAFFICLIGHT_H

#include <mutex>
#include <deque>
#include <random>
#include <condition_variable>
#include "TrafficObject.h"

// forward declarations to avoid include cycle
class Vehicle;

enum class TrafficLightPhase { red, green };

template <class T>
class MessageQueue
{
public:
    void send(T&& message);
    T receive();

private:
    std::deque<TrafficLightPhase> _queue;
    std::condition_variable _condVar;
    std::mutex _mutex;
};

class TrafficLight : TrafficObject
{
public:
    // constructor / desctructor
    TrafficLight();

    // getters / setters
    TrafficLightPhase getCurrentPhase();

    // typical behaviour methods
    void waitForGreen();
    void simulate();

private:
    // typical behaviour methods
    void cycleThroughPhases();

    MessageQueue<TrafficLightPhase> _msgQueue;
    TrafficLightPhase _currentPhase;
    std::mt19937 _rng;
    std::condition_variable _condition;
    std::mutex _mutex;
};

#endif
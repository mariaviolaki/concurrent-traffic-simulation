#include <iostream>
#include <random>
#include "TrafficLight.h"

/* Implementation of class "MessageQueue" */

// Add new message to the queue and send a notification
template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    // Add message to queue under the lock to prevent a data race
    std::lock_guard<std::mutex> lock(_mutex);
    _queue.emplace_back(std::move(msg));
    // Wake up consumer thread
    _condVar.notify_one();
}

// Wait and receive new message - pull it from the queue before returning it
template <typename T>
T MessageQueue<T>::receive()
{
    // Prevent data race with a lock that can be passed to the condition variable
    std::unique_lock<std::mutex> lock(_mutex);
    // Pause thread execution until queue is no longer empty
    _condVar.wait(lock, [this](){ return !_queue.empty(); });

    // Store last element before removing it from the queue
    T message = std::move(_queue.back());
    _queue.pop_back();

    return message;
}


/* Implementation of class "TrafficLight" */

TrafficLight::TrafficLight() : _currentPhase(TrafficLightPhase::red), _rng(std::random_device{}()) {}

TrafficLightPhase TrafficLight::getCurrentPhase() { return _currentPhase; }

// Method "cycleThroughPhases" starts in a thread, using the thread queue in the base class
void TrafficLight::simulate()
{
    threads.emplace_back(&TrafficLight::cycleThroughPhases, this);
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    // Set start time
    std::chrono::high_resolution_clock::time_point timeStart = std::chrono::high_resolution_clock::now();
    // Set time cycle range (in milliseconds)
    std::uniform_int_distribution<int> dist(4000, 6000);

    while (true)
    {
        // Assign a random number to the cycle duration
        int randomDuration = dist(_rng);
        // Set second time point and calculate difference
        auto timePoint = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(timePoint - timeStart).count();

        // Toggle red/green traffic light phase
        if (duration >= randomDuration)
        {
            if (_currentPhase == TrafficLightPhase::red)
                _currentPhase = TrafficLightPhase::green;
            else
                _currentPhase = TrafficLightPhase::red;

            // Send new traffic light phase to message queue
            _msgQueue.send(std::move(_currentPhase));
            // Set new time starting point
            timeStart = std::chrono::high_resolution_clock::now();
        }

        // Wait 1ms to reduce iterations (and load on the CPU)
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

void TrafficLight::waitForGreen()
{
    // Run infinite while-loop and repeatedly call the receive function on the message queue
    while (true)
    {
        TrafficLightPhase phase = _msgQueue.receive();
        // Return after receiving TrafficLightPhase::green
        if (phase == TrafficLightPhase::green)
            return;
    }
}

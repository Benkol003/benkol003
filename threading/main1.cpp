#include <cmath>
#include <cstdlib>
#include <cstddef>
#include <ctime>
#include <memory>
#include <variant>
#include <vector>
#include <utility>
#include <thread>
#include <stdexcept>
#include <iostream>
#include <barrier>
#include <memory>
#include <functional>
#include <mutex>

#include <benchmark/benchmark.h>


inline float random_float() {
    return static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX);
}

namespace together {

    class Car {
    public:
        Car(std::string driver_, float speed_, char colour_, std::string model_) : driver(driver_), speed(speed_), colour(colour_), model(model_) {}
        float update_speed(int velocity) { speed += velocity; return speed; }

    private:
        std::string driver;
        float speed;
        char colour;
        std::string model;
    }; // Car

/*static void update_speed(Car &car) {
  car.speed += car.velocity;
}*/

// Create an array of cars to benchmark
    static std::vector<Car> car_builder(size_t count) {
        std::vector<Car> cars;
        cars.reserve(count);
        for (size_t i = 0; i < count; i++) {
            cars.emplace_back(Car{"Coopo",        // Driver
                                  random_float(), // speed
                                  'R',            // colour
                                  "Fiat"});       // model
        }
        return cars;
    }

} // namespace together

namespace components {
    class Cars {
    public:
        Cars(size_t count){
            drivers.reserve(count);
            speeds.reserve(count);
            colours.reserve(count);
            models.reserve(count);
        }

        float update_speeds(float velocity) {
            for(auto &speed : speeds) {
                speed += velocity;
            }
            return velocity;
        }

        std::vector<std::string> drivers;
        std::vector<float> speeds;
        std::vector<char> colours;
        std::vector<std::string> models;
    }; // Cars



    static Cars cars_builder(size_t count) {
        Cars cars{count};
        for (size_t i = 0; i < count; i++) {
            cars.drivers.emplace_back("Coopo");
            cars.speeds.emplace_back(random_float());
            cars.speeds.emplace_back('R');
            cars.models.emplace_back("Fiat");
        }

        return cars;
    }
} // namespace components

constexpr size_t count = 100000;

static void coopo(benchmark::State &state) {
    std::vector<together::Car> cars = together::car_builder(count);
    float velocity = random_float();
    for (auto _ : state) {
        for (auto c : cars){
            c.update_speed(velocity);
        }
    }
}

static void ijan1(benchmark::State &state) {
    components::Cars cars = components::cars_builder(count);
    float velocity = random_float();
    for (auto _ : state) {
        cars.update_speeds(velocity);
    }
}


//-----------------------------------------

//see https://www.youtube.com/watch?v=A7sVFJLJM-A&t=655s for a guid on threading

struct CarsComponents {
    std::vector<float> speed;
    std::vector<char> color;
    std::vector<std::string> driver;
    std::vector<std::string> model;

    std::vector<std::jthread> speedThrds;
    std::barrier<> speedTBR;
    uint maxThreads;


    CarsComponents(uint max_threads, const size_t count) : maxThreads(max_threads), speedTBR(max_threads+1) {
        //note we do max_threads+1 as we need to account for the current thread sync-ing on the barrier aswell
        speed.reserve(count);
        color.reserve(count);
        driver.reserve(count);
        model.reserve(count);
        speedThrds.reserve(max_threads);
        }

        void dispatchThreads(size_t count){ //only call once

        uint slice_size = this->speed.size()/maxThreads;
        if(slice_size==0) throw std::runtime_error("Too many Threads!");
        uint slice1_size = this->speed.size() - (maxThreads * slice_size);

        float rnd = random_float();
        speedThrds.emplace_back(&CarsComponents::speedUpdater,this,0, slice1_size,rnd);
        assert(speedThrds[0].joinable());
        for(int i=1; i<maxThreads; ++i){
            speedThrds.emplace_back(&CarsComponents::speedUpdater,this,slice1_size+(slice_size*i),slice1_size+(slice_size*(i+1)), rnd);
            assert(speedThrds[i].joinable());
        }

    }

    void speedUpdater(std::stop_token stoken, int i, int j, float velocity){
        while(!stoken.stop_requested()){
            for(;i<j; ++i){
                this->speed[i]=velocity;
            }
            this->speedTBR.arrive_and_wait(); //synchronise writes every frame
        }
        this->speedTBR.arrive_and_drop();
    }

};

static void CarsBuilder(CarsComponents& cars, size_t count){
    for(int i=0;i<count; ++i){
        cars.speed.push_back(random_float());
        cars.color.push_back('R');
        cars.driver.emplace_back("UnladenCoconut");
        cars.model.emplace_back("Lada");
    }
    cars.dispatchThreads(count);
}

static void UnladenCoconut(benchmark::State &state){
    uint max_threads = std::thread::hardware_concurrency()-1; //current thread aswell
    CarsComponents cars(max_threads, count);
    CarsBuilder(cars, count);
    float velocity_change=random_float();
    for(auto _: state){
        cars.speedTBR.arrive_and_wait();
    }

    cars.speedTBR.arrive_and_drop();
    /*
     * the issue here is that all other threads need to wait at the barrier; main needs to wait for this to happen
     * then it can request a stop before allowing others to proceed
     * otherwise, other threads will re-enter the loop and hang on the barrier wait, whilst others will
     * exit
     */
    for(int i=0; i<cars.speedThrds.size();++i){
        cars.speedThrds[i].request_stop();
        cars.speedThrds[i].join();

    }
}
//-----------------------------------------
BENCHMARK(UnladenCoconut)->Unit(benchmark::kNanosecond);
BENCHMARK(ijan1)->Unit(benchmark::kNanosecond);

int main(int argc, char** argv) {
    benchmark::Initialize(&argc, argv);
    benchmark::RunSpecifiedBenchmarks();
}
//
// Created by UnladenCoconut on 01/04/23.
//
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

constexpr int count = 80000;

static void coopo(benchmark::State &state) {
    std::vector<together::Car> cars = together::car_builder(count);
    float velocity = random_float();
    for (auto _ : state) {
        for (auto c : cars){
            benchmark::DoNotOptimize(c.update_speed(velocity));
        }
    }
}

static void ijan1(benchmark::State &state) {
    components::Cars cars = components::cars_builder(count);
    float velocity = random_float();
    for (auto _ : state) {
        benchmark::DoNotOptimize(cars.update_speeds(velocity));
    }
}


//-----------------------------------------
struct CarsComponents {
    std::vector<float> speed;
    std::vector<char> color;
    std::vector<std::string> driver;
    std::vector<std::string> model;

    CarsComponents(const int count) {
        speed.reserve(count);
        color.reserve(count);
        driver.reserve(count);
        model.reserve(count);
    }

    void updateSpeeds(float velocity_change){
        for(int i=0; i<this->speed.size();++i){
            this->speed[i]=velocity_change;
        }
    }

};

static CarsComponents CarsBuilder(size_t count){
    CarsComponents cars(count);
    for(int i=0;i<count; ++i){
        cars.speed.push_back(random_float());
        cars.color.push_back('R');
        cars.driver.emplace_back("UnladenCoconut");
        cars.model.emplace_back("Lada");
    }
    return std::move(cars);
}

static void UnladenCoconut(benchmark::State &state){
    CarsComponents cars = CarsBuilder(count);
    float velocity_change=random_float();
    for(auto _: state){
        cars.updateSpeeds(velocity_change);
    }
}
//-----------------------------------------
BENCHMARK(UnladenCoconut);
BENCHMARK(ijan1);

int main(int argc, char** argv) {
    ::benchmark::Initialize(&argc, argv);
    ::benchmark::RunSpecifiedBenchmarks();
}
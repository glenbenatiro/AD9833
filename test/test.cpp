#include <array>
#include <iostream>
#include <chrono>
#include <thread>

#include <pigpio.h>

#include "../AD9833.h"

class AD9833_c : public AD9833 
{
    private:
        int m_fd;

    private:
        void spi_xfer (char* rxd, char* txd, unsigned length)
        {
            spiXfer (m_fd, txd, rxd, length);
        }

    public:
        AD9833_c ()
        {
            gpioInitialise ();

            m_fd = spiOpen (0, 100'000, 2);
        }

       ~AD9833_c ()
       {
            gpioTerminate ();
       }
};

int main ()
{
    AD9833_c func_gen;

    std::array<AD9833_c::WAVE_TYPE, 3> wave_arr = 
    {
        AD9833_c::WAVE_TYPE::SINE,
        AD9833_c::WAVE_TYPE::TRIANGLE,
        AD9833_c::WAVE_TYPE::SQUARE
    };

    std::array<double, 7> freq_arr = {1, 10, 100, 1'000, 10'000, 100'000, 1'000'000};

    while (true)
    {
        for (int a = 0; a < 3; a++)
        {
            std::cout << "set wave type: " << static_cast<int>(wave_arr[a]) << "\n";

            func_gen.wave_type (wave_arr[a]);

            for (size_t x = 0; x < freq_arr.size (); x++)
            {
                std::cout << "set freq to: " << freq_arr[x] << "\n";

                func_gen.frequency (freq_arr[x]);

                std::this_thread::sleep_for (std::chrono::milliseconds (1000));
            }
        }
    }
}
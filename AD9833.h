#ifndef AD9833_H
#define AD9833_H

#include <cstdint>

// https://www.analog.com/media/en/technical-documentation/data-sheets/ad9833.pd
class AD9833
{
  public:
    enum class WAVE_TYPE
    {
      SINE,
      TRIANGLE,
      SQUARE,
      SQUARE_HALF,
      SQUARE_FULL,
      DC
    };

  private:
    static constexpr double   REFERENCE_CLOCK_FREQUENCY = 25'000'000.0;
    static constexpr unsigned POW_2_28                  = 268'435'456;
    
    // AD9833 Control Register
    bool      m_B28       = 0, 
              m_HLB       = 0,
              m_FSELECT   = 0,
              m_PSELECT   = 0,
              m_Reset     = 0,
              m_SLEEP1    = 0,
              m_SLEEP12   = 0,
              m_OPBITEN   = 0,
              m_DIV2      = 0,
              m_Mode      = 0;
    
    char      m_txbuff[2] = {0x00, 0x00},
              m_rxbuff[2] = {0x00, 0x00};
    
    double    m_frequency = 100'000;
    double    m_phase     = 0.0;

    WAVE_TYPE m_wave_type = WAVE_TYPE::DC;

  protected:
    unsigned  m_CS,
              m_MISO,
              m_MOSI,
              m_SCLK,
              m_SPI_channel;
    
    double    m_baud;
  
  public:
    static constexpr double MAX_FREQUENCY = 12'500'000.0;
    static constexpr double MIN_FREQUENCY = 0.1; 
    static constexpr double TYP_AMPLITUDE = 0.65;  // V, sine and triangle
    static constexpr double MAX_AMPLITUDE = 5.00;    // V, square
    static constexpr double MIN_AMPLITUDE = 0.26;  // V, off (reset)

  private:
    void init             ();
    void update_ctrl_reg  ();
    void write_reg        ();
    void sel_freq_reg     (bool value);
    void sel_phase_reg    (bool value);
    void set_HLB          (bool value);
    void set_write_mode   (bool value);
  
  protected:
    virtual void spi_xfer (char* rxd, char* txd, unsigned length) = 0;

  public: 
    AD9833    ();  
   ~AD9833    ();

    void    run           ();
    void    stop          ();
    void    reset         ();
    void    unreset       ();
    void    wave_type     (WAVE_TYPE value);
    void    frequency     (double value);
    void    period        (double value);
    void    phase         (double value, bool phase_reg = 0);
    double  amplitude     ();
};

#endif 
#include "AD9833.h"

#include <thread>
#include <cmath>

AD9833::
AD9833 ()
{

}

AD9833::
~AD9833 ()
{

}

void AD9833:: 
update_ctrl_reg ()
{
  uint16_t cmd =  0         << 15 |
                  0         << 14 |
                  m_B28     << 13 |
                  m_HLB     << 12 |
                  m_FSELECT << 11 |
                  m_PSELECT << 10 |
                  0         <<  9 |
                  m_Reset   <<  8 |
                  m_SLEEP1  <<  7 |
                  m_SLEEP12 <<  6 |
                  m_OPBITEN <<  5 | 
                  0         <<  4 |
                  m_DIV2    <<  3 |
                  0         <<  2 |
                  m_Mode    <<  1 |
                  0         <<  0 ;
  
  // most significant byte first according to datasheet
  m_txbuff[1] = cmd & 0x00FF;
  m_txbuff[0] = (cmd >> 8) & 0x00FF;

  write_reg ();
}

void AD9833::
write_reg ()
{
  spi_xfer (m_rxbuff, m_txbuff, 2);

  std::this_thread::sleep_for (std::chrono::microseconds (10));
}

void AD9833::
sel_freq_reg (bool value)
{
  m_FSELECT = value;
  update_ctrl_reg ();
}

void AD9833::
sel_phase_reg (bool value)
{
  m_PSELECT = value;
  update_ctrl_reg ();
}

void AD9833::
set_HLB (bool value)
{
  m_HLB = value;
  update_ctrl_reg ();
}

void AD9833::
set_write_mode (bool value)
{
  // 1 = complete write through 2 consecutive writes
  // 0 = freq reg is split into MSB and LSB
  m_B28 = value;
  update_ctrl_reg ();
}

void AD9833:: 
run ()
{
  unreset ();
}

void AD9833:: 
stop ()
{
  reset ();
}

void AD9833:: 
reset ()
{
  m_Reset = 1;
  update_ctrl_reg ();
}

void AD9833:: 
unreset ()
{
  m_Reset = 0;
  update_ctrl_reg ();
}

void AD9833:: 
wave_type (WAVE_TYPE value)
{
  switch (value)
  {
    case WAVE_TYPE::SINE:
    {  
      m_OPBITEN = 0;
      m_Mode    = 0;

      break;
    }

    case WAVE_TYPE::TRIANGLE:
    {  
      m_OPBITEN = 0;
      m_Mode    = 1;
    
      break;
    }

    case WAVE_TYPE::SQUARE_HALF:
    {  
      m_OPBITEN = 1;
      m_Mode    = 0;
      m_DIV2    = 0;
    
      break;
    }

    case WAVE_TYPE::SQUARE:
    case WAVE_TYPE::SQUARE_FULL:
    {  
      m_OPBITEN = 1;
      m_Mode    = 0;
      m_DIV2    = 1;
      
      break;
    }   
    
    case WAVE_TYPE::DC:
    {
      wave_type (WAVE_TYPE::SINE);
      stop ();

      break;
    }
  }

  if (value != WAVE_TYPE::DC)
  {
    update_ctrl_reg ();
    run             ();
  }  
}

void AD9833:: 
frequency (double value)
{
  if ((value <= MAX_FREQUENCY) && (value >= MIN_FREQUENCY))
  {
    uint32_t divider = ((value * POW_2_28) / REFERENCE_CLOCK_FREQUENCY);
    divider &= 0x0FFFFFFF;

    if (m_B28 == 1)
    {
      uint16_t upper  = (m_FSELECT + 1) << 14 | ((divider >> 14) & 0x3FFF);
      uint16_t lower  = (m_FSELECT + 1) << 14 | (divider & 0x3FFF);
      
      m_txbuff[0] = (lower >> 8) & 0xFF;
      m_txbuff[1] = lower & 0xFF;
      write_reg ();

      m_txbuff[0] = (upper >> 8) & 0xFF;
      m_txbuff[1] = upper & 0xFF;
      write_reg ();
    } 
    else if (m_B28 == 0) 
    {
      uint16_t upper  = (m_FSELECT + 1) << 14 | ((divider >> 14) & 0x3FFF);
      uint16_t lower  = (m_FSELECT + 1) << 14 | (divider & 0x3FFF);

      set_HLB (0); 
      m_txbuff[0] = (lower >> 8) & 0xFF;
      m_txbuff[1] = lower & 0xFF;
      write_reg ();

      set_HLB (1);
      m_txbuff[0] = (upper >> 8) & 0xFF;
      m_txbuff[1] = upper & 0xFF;
      write_reg ();
    }

    m_frequency = REFERENCE_CLOCK_FREQUENCY / divider;
  }
}

void AD9833:: 
period (double value)
{
  frequency (1.0 / value);
}

void AD9833:: 
phase (double value, bool phase_reg)
{
  uint16_t phase  = (std::fmod (value, 360.0) * 4096) / (2 * 3.141592653);
  uint16_t data   = (3 << 14) | (phase_reg << 13) | (phase & 0xFFF);

  m_txbuff[0] = (data >> 8) & 0xFF;
  m_txbuff[1] = data & 0xFF;

  write_reg ();
}

// EOF
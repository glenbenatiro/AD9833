#include "AD9833.h"

#include <cstdio>
#include <iostream>

AD9833::
AD9833 ()
{

}

void AD9833:: 
update_ctrl_reg ()
{
  m_txbuff[0] = 0x00 | (m_B28 << 5) | (m_HLB << 4) | (m_FSELECT << 3) |
    (m_PSELECT << 2) | m_Reset;
        
  m_txbuff[1] = 0x00 | (m_SLEEP1 << 7) | (m_SLEEP12 << 6) | 
    (m_OPBITEN << 5) | (m_DIV2 << 3) | (m_Mode << 1);

  write_reg ();
}

void AD9833::
write_reg ()
{
  spi_xfer (m_rxbuff, m_txbuff, 2);
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
  m_Reset = 0;
  update_ctrl_reg ();
}

void AD9833:: 
stop () 
{
  m_Reset = 1;
  update_ctrl_reg ();
}

void AD9833:: 
wave_type (WAVE_TYPE value)
{
  switch (value)
  {
    case WAVE_TYPE::SINE:
      m_OPBITEN = 0;
      m_DIV2 = 0;
      m_Mode = 0;
      break;
    case WAVE_TYPE::TRIANGLE:
      m_OPBITEN = 0;
      m_DIV2 = 0;
      m_Mode = 1;
      break;
    case WAVE_TYPE::SQUARE_FULL:
      m_OPBITEN = 1;
      m_DIV2 = 1;
      m_Mode = 0;
      break;
    case WAVE_TYPE::SQUARE_HALF:
      m_OPBITEN = 1;
      m_DIV2 = 0;
      m_Mode = 0;
      break;
    case WAVE_TYPE::SQUARE:
      wave_type (WAVE_TYPE::SQUARE_FULL);
      break;
    case WAVE_TYPE::DC:
      // add here code for DC signal
      break;
    default:
      wave_type (WAVE_TYPE::SINE);
      break;
  }

  update_ctrl_reg ();
}

void AD9833:: 
frequency (double frequency)
{
  if ((frequency <= MAX_FREQUENCY) && (frequency >= MIN_FREQUENCY))
  {
    uint32_t divider = ((frequency * POW_2_28) / 
    static_cast<double>(REFERENCE_CLOCK_FREQUENCY));
    divider &= 0x0FFFFFFF;

    if (m_B28 == 1)
    {
      m_txbuff[0] = 0x00 | ((m_FSELECT + 1) << 6) | ((divider >> 8) & 0x3F);
      m_txbuff[1] = divider & 0xFF;
      write_reg ();

      divider >>= 14;

      m_txbuff[0] = 0x00 | ((m_FSELECT + 1) << 6) | ((divider >> 8) & 0x3F);
      m_txbuff[1] = divider & 0xFF;
      write_reg ();
    } 
    else if (m_B28 == 0)
    {
      set_HLB (0);
      m_txbuff[0] = 0x00 | ((m_FSELECT + 1) << 6) | ((divider >> 8) & 0x3F);
      m_txbuff[1] = divider & 0xFF;
      write_reg ();

      divider >>= 14;

      set_HLB (1);
      m_txbuff[0] = 0x00 | ((m_FSELECT + 1) << 6) | ((divider >> 8) & 0x3F);
      m_txbuff[1] = divider & 0xFF;
      write_reg ();
    }
  }
}

void AD9833:: 
period (double value)
{
  frequency (1.0 / value);
}

void AD9833:: 
phase (double phase)
{

}

// EOF
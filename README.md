# Digital Stopwatch & Countdown Timer — Embedded C (ATmega32)

A hardware stopwatch and countdown timer built in **Embedded C** on an **ATmega32** microcontroller. Displays time on **six 7-segment displays** using multiplexing, with dual-mode operation, hardware interrupt controls, and manual time adjustment via push buttons.

---

## Features

### Dual Operation Modes
| Mode | Behavior |
|------|----------|
| **Stopwatch (Count Up)** | Counts up from 00:00:00, yellow LED indicator |
| **Countdown Timer** | Counts down to 00:00:00, buzzer fires on zero, red LED indicator |

Toggle between modes using a dedicated button (PB7) — mode switches instantly even while running.

### Controls (Hardware Interrupts)
| Button | Pin | Trigger | Action |
|--------|-----|---------|--------|
| Reset | PD2 | INT0 (falling edge) | Reset all digits to 00:00:00 |
| Pause | PD3 | INT1 (falling edge) | Freeze the timer |
| Resume | PB2 | INT2 (falling edge) | Resume counting |

### Manual Time Adjustment (while paused)
| Button | Pin | Action |
|--------|-----|--------|
| PB0 | Hour − | Decrease hours |
| PB1 | Hour + | Increase hours |
| PB3 | Min − | Decrease minutes |
| PB4 | Min + | Increase minutes |
| PB5 | Sec − | Decrease seconds |
| PB6 | Sec + | Increase seconds |

---

## Hardware Design

| Component | Purpose |
|-----------|---------|
| ATmega32 @ 16 MHz | Main microcontroller |
| 6× 7-Segment Displays | Time display (HH:MM:SS) |
| Buzzer | Alert when countdown reaches zero |
| 2× LEDs (Yellow/Red) | Mode indicator |
| Push Buttons | Reset, pause, resume, time adjust |

**Multiplexing:** All 6 displays share the same data lines (PORTC lower nibble). PORTA bits 0–5 activate each display one at a time with 1 ms per digit, cycling fast enough to appear simultaneous to the human eye.

---

## Peripheral Configuration

```
Timer1  → CTC mode, OCR1A = 15624 → 1 Hz tick @ 16 MHz (prescaler 1024)
INT0    → Falling edge (PD2, pull-up) → Reset ISR
INT1    → Falling edge (PD3, pull-up) → Pause ISR
INT2    → Falling edge (PB2, pull-up) → Resume ISR
PORTA   → Output: 7-segment enable lines (bits 0–5)
PORTC   → Output: BCD digit data (bits 0–3)
PORTD   → PD0=Buzzer, PD4=Yellow LED, PD5=Red LED
PORTB   → Input: all buttons (pull-up enabled)
```

---

## Project Structure

```
stopwatch/
└── stopwatch.c     # Complete single-file implementation
```

**Concepts demonstrated:**
- Timer/Counter in CTC mode for precise 1 Hz timing
- External hardware interrupts (INT0, INT1, INT2)
- 7-segment multiplexed display driving
- One-shot button debounce using flags
- ISR-driven state machine (pause/resume/reset)
- Buzzer alert on countdown completion

---

## Tools & Environment

| Tool | Purpose |
|------|---------|
| AVR-GCC / Eclipse | Compiler & IDE |
| ATmega32 @ 16 MHz | Target MCU |
| Proteus (optional) | Simulation |

---

## Author

**Mohamed Baiomy Abdelkader**  
Mechatronics & Robotics Engineering Student — Ain Shams University  
[LinkedIn](https://linkedin.com/in/mohamed-baiomy) · [GitHub](https://github.com/mb4871787-creator)

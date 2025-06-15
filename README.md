# STM32F103 Digital Oscilloscope Design

A design study for implementing a real-time digital oscilloscope on resource-constrained embedded systems.

## Project Overview

**Goal**: Create a functional digital oscilloscope that demonstrates real-time signal processing on STM32F103ZET6
**Constraints**: 64KB RAM, 512KB Flash, 72MHz ARM Cortex-M3
**Challenge**: Balance performance, user experience, and code maintainability

## Core Design Challenges

### 1. Real-Time Display Performance Problem

**Challenge**: How to achieve smooth waveform display on a slow LCD interface while maintaining real-time sampling?

**Traditional Approach Issues**:
- Full-screen refresh: Too slow, causes flicker
- Double buffering: Requires too much RAM (480×800×2 = 768KB > 64KB available)
- Simple point plotting: Disconnected, hard to read waveforms

**Our Solution - Incremental Sweep Rendering**:
- **Why**: Mimics real oscilloscope behavior, uses minimal memory
- **How**: Only redraw the current vertical "sweep line", preserve previous traces
- **Trade-off**: Slight visual artifacts vs. real-time performance

**Key Insight**: Instead of thinking like a computer display, think like an analog oscilloscope CRT.

### 2. Automatic Timebase Problem

**Challenge**: How to automatically adjust display timebase without user input?

**Problem Analysis**:
- Users shouldn't need to manually set timebase (poor UX)
- Signal frequency is unknown at runtime
- Need to show "useful" number of periods (not too dense, not too sparse)

**Solution Strategy - Adaptive Period Detection**:
- **Step 1**: Detect signal characteristics in real-time
- **Step 2**: Calculate optimal display timebase
- **Step 3**: Adjust smoothly without disrupting display

**Algorithm Choice - Rising Edge Detection**:
- **Why not zero-crossing**: Sensitive to noise, offset issues
- **Why not FFT**: Too computationally expensive, needs large buffers
- **Why rising edge**: Robust, works with any amplitude/offset

**Design Decision**: Target 4-6 periods on screen (empirically good for visualization)

### 3. System Architecture Problem

**Challenge**: How to organize code for maintainability while meeting real-time constraints?

**Analysis of Requirements**:
- Real-time sampling (cannot be interrupted)
- User interface (can tolerate some delay)
- Display rendering (needs to be efficient)
- Parameter control (infrequent operations)

**Architecture Decision - Functional Separation**:
```
Real-time Layer:    Timer ISR + Main Loop (sampling, signal processing)
Display Layer:      Incremental rendering (oscilloscope.c)
Interface Layer:    User controls (buttons.c)
System Layer:       Coordination (main.c)
```

**Why this separation**:
- **Testability**: Each module can be tested independently
- **Maintainability**: Clear responsibilities, low coupling
- **Performance**: Critical path (sampling) isolated from UI logic

### 4. Memory Management Strategy

**Challenge**: Efficient use of limited 64KB RAM without dynamic allocation

**Problem**: Embedded systems should avoid malloc/free due to:
- Fragmentation risk
- Non-deterministic timing
- Limited debugging tools

**Solution - Static Allocation Strategy**:
- All variables pre-allocated at compile time
- State machines instead of recursive algorithms
- Minimal stack usage (avoid deep function calls)

**Design Pattern**: "Global state with functional interfaces"
- Global variables for system state (accessible across modules)
- Functions for operations (stateless, predictable)

### 5. Signal Generation Philosophy

**Challenge**: Create test signals that demonstrate oscilloscope capabilities

**Requirements Analysis**:
- Predictable waveform (for verification)
- Adjustable parameters (frequency, amplitude)
- Continuous operation (no gaps or glitches)

**Design Choice - Triangle Wave**:
- **Why not sine**: Complex math, needs lookup tables
- **Why not square**: Too simple, doesn't test analog performance
- **Why triangle**: Linear ramps, easy to generate, good for testing

**Parameter Control Strategy**:
- Frequency: Use divider instead of recalculating step size (simpler)
- Amplitude: Dynamic range calculation (more flexible)
- Phase: Continuous generation (no restart artifacts)

## Engineering Trade-offs

### Performance vs. Features
**Decision**: Prioritize real-time performance over advanced features
**Rationale**: Core oscilloscope functionality must work reliably first

### Memory vs. Processing
**Decision**: Use CPU cycles to save RAM
**Example**: Recalculate display coordinates instead of caching them

### Simplicity vs. Flexibility
**Decision**: Simple, fixed algorithms over configurable parameters
**Rationale**: Easier to debug, more predictable behavior

## User Experience Design Thinking

### Interface Philosophy
**Problem**: Complex instruments have complex interfaces (bad for embedded)
**Solution**: "Two-button interface" - minimal learning curve

**Design Principle**: "Direct manipulation"
- Select what you want to change (KEY0)
- Change it (KEY1)
- See immediate feedback (display updates)

### Visual Design Decisions

**Color Coding Strategy**:
- Blue: DAC output (input to system)
- Red: ADC input (output from system)
- Yellow: User selection highlight
- Why: Standard oscilloscope channel colors

**Layout Philosophy**:
- Top: DAC signal (source)
- Bottom: ADC signal (measured)
- Why: Natural signal flow visualization

## Algorithm Design Rationale

### Period Detection Algorithm

**Challenge**: Detect signal period in noisy, real-time environment

**Considered Approaches**:
1. **Zero-crossing**: Simple but sensitive to DC offset and noise
2. **Peak detection**: Works but needs threshold tuning
3. **Auto-correlation**: Mathematically robust but computationally expensive
4. **Template matching**: Accurate but requires known signal shape

**Chosen Approach - Threshold Crossing with Hysteresis**:
- Robust to noise (threshold)
- Insensitive to DC offset (relative to signal center)
- Computationally efficient (simple comparison)
- Self-calibrating (adapts to signal amplitude)

### Timebase Adjustment Strategy

**Problem**: When and how much to adjust timebase?

**Design Philosophy**: "Smooth adaptation"
- Small, gradual changes rather than sudden jumps
- Hysteresis to prevent oscillation
- Clear target: 5 periods visible

**Implementation Strategy**:
- Measure period in screen pixels
- Compare to target width
- Adjust timebase incrementally

## Development Process Insights

### Debugging Strategy
**Challenge**: Limited debugging tools on embedded systems
**Solution**: Multi-layer debugging approach
- Serial output for real-time data
- LED indicators for state
- LCD display for visual feedback

### Testing Methodology
**Challenge**: How to verify real-time system behavior?
**Approach**: 
- Controlled test signals (known triangle wave)
- Visual verification (waveform display)
- Quantitative measurement (serial output)

### Code Organization Evolution
**Initial**: Everything in main.c (rapid prototyping)
**Problem**: Became unmaintainable, hard to debug
**Solution**: Functional decomposition into modules
**Result**: Clear interfaces, easier testing

## Lessons Learned

### 1. Real-Time Systems Design
- **Interrupt discipline**: Keep ISRs minimal, defer work to main loop
- **State management**: Global state with functional interfaces works well
- **Performance**: Measure early, optimize based on actual bottlenecks

### 2. User Interface Design
- **Feedback loops**: User actions must have immediate, visible results
- **Consistency**: Similar operations should behave similarly
- **Forgiveness**: System should recover gracefully from user errors

### 3. Embedded Programming
- **Resource awareness**: Every byte of RAM and flash matters
- **Deterministic behavior**: Prefer predictable algorithms over optimal ones
- **Debugging**: Build in observability from the start

## Future Enhancement Strategies

### Scalability Considerations
- **More channels**: Current architecture supports easy addition
- **Higher sample rates**: May need DMA, different display strategy
- **Advanced triggers**: Would require separate trigger detection module

### Performance Optimization Opportunities
- **DMA for ADC**: Reduce CPU load, enable higher sample rates
- **Hardware timers for DAC**: More precise signal generation
- **Assembly optimization**: Critical display routines could be hand-optimized

### User Experience Improvements
- **Touch interface**: Natural for parameter adjustment
- **Persistent settings**: Remember user preferences
- **Measurement cursors**: Quantitative analysis capabilities

## Design Philosophy Summary

This project demonstrates that effective embedded systems design is about:

1. **Understanding constraints** - Work with hardware limitations, not against them
2. **Choosing appropriate algorithms** - Simple, robust solutions often beat complex ones
3. **Layered architecture** - Separate concerns for maintainability
4. **User-centered design** - Interface should match user mental model
5. **Iterative development** - Build core functionality first, add features incrementally

The result is a system that achieves its core objectives within constraints while maintaining code quality and user experience.
# Bethel University Sensor Suite Code 24/25

For part C in the [High Thrust Challenge from the Midwest Rocketry Competition 2024/2025](https://dept.aem.umn.edu/mnsgc/Space_Grant_Midwest_Rocketry_Competition_2024_2025/Midwest_Rocketry_Competition_Handbook_2024-2025_v5.pdf#page=4). Includes the optional challenge of using a Pitot Tube.  

Deduces altitude, speed, and acceleration during a High-Powered Rocket's ascent in 4 independent methods (barometric pressure, 3-axis acceleration, gps, differential pressure)

## Architecture

quick overview of architecture

### Dataflow

quick explanation of dataflow, and some details

```mermaid
graph LR
    sensors[Sensors] --> preprocess[Acquire and Annotate] --> queue[Queue] --> calculate[Calculate Kinematics] --> store[Store Data]
```

### Core Allocation

quick explanation of why core allocation

#### Core Responsibilities

quick overview of what each core does

```mermaid
graph TD
    subgraph core1[Core 1]
        queue1[Read From Queue]
        calculate[Calculate Kinematics]
        write0[Save To SD]
        write1[Save To Flash]
    end

    subgraph core0[Core 0]
        acquire[Acquire Data]
        annotate[Annotate Sample]
        queue0[Write To Queue]
    end
```

### Inter Core Communication

quick overview of how the queue ties everything together

```mermaid
graph LR
        core0[Core 0]
        queue[Queue]
        core1[Core 1]

    core0 --> queue --> core1
```

## Optimizations

### Multicore

#### Thread-Safe Circular Queue

### Custom Sensor Drivers

## Limitations

### Modifying Sensors Used

### Single Core Flash Writing

### Core Throttling

## Retrospective

### Writing Sensor Drivers

#### Reading Datasheets

### Repeated Code

### Muddled Responsibilities

## Lite User Guide

explain this is meant to give context so the reader knows more about design decisions

### Annotated Sample Format

### Sample Accuracy

### Building
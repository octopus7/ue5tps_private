```mermaid
stateDiagram-v2
    [*] --> A
    A --> B: event
    B --> [*]: donestateDiagram-v2
    [*] --> Free
    Free --> SnapIn: CoverInput && ValidCoverLine
    SnapIn --> InCover: AlignDone
    InCover --> PeekSideLeft: Aim && NearLeftOpen
    InCover --> PeekSideRight: Aim && NearRightOpen
    InCover --> PeekOver: Aim && IsLowCover
    PeekSideLeft --> InCover: StopAim
    PeekSideRight --> InCover: StopAim
    PeekOver --> InCover: StopAim
    InCover --> Exit: BackInput || Sprint || CoverInput
    Exit --> Free: End

class StateOutdated(RuntimeError):
    """
    Indicates that a state update failed because the revision number was too old.
    """
    def __init__(self, state):
        self.state = state
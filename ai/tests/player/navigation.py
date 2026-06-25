
from player.navigation import navigate

def test_already_on_target():
    assert navigate((2, 2), 0, (2, 2), (10, 10)) == []

def test_move_to_north():
    assert navigate((2, 4), 0, (2, 2), (10, 10)) == ["Forward", "Forward"]

def test_turn_and_move():
    assert navigate((2, 2), 0, (5, 2), (10, 10)) == ["Right", "Forward", "Forward", "Forward"]

def test_move_on_2_axis():
    assert navigate((2, 2), 0, (5, 4), (10, 10)) == [
        "Right", "Forward", "Forward", "Forward",
        "Right", "Forward", "Forward"
    ]

def test_edge_map_X():
    assert navigate((8, 0), 0, (1, 0), (10, 10)) == ["Right", "Forward", "Forward", "Forward"]

def test_edge_map_Y():
    assert navigate((0, 9), 2, (0, 1), (10, 10)) == ["Forward", "Forward"]

def test_half_turn():
    assert navigate((0, 5), 2, (0, 3), (10, 10)) == ["Right", "Right", "Forward", "Forward"]
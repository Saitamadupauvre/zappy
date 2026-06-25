
import pytest
from parser.args import parse_args

def test_nominal():
    args = parse_args(["-p", "4242", "-n", "team1", "-h", "localhost"])
    assert args.port == 4242
    assert args.name == "team1"
    assert args.host == "localhost"

def test_default_host():
    args = parse_args(["-p", "4242", "-n", "team1"])
    assert args.host == "localhost"

def test_port_is_int():
    args = parse_args(["-p", "8080", "-n", "team1"])
    assert isinstance(args.port, int)

def test_port_too_low():
    with pytest.raises(SystemExit):
        parse_args(["-p", "0", "-n", "team1"])

def test_port_too_high():
    with pytest.raises(SystemExit):
        parse_args(["-p", "99999", "-n", "team1"])

def test_missing_port():
    with pytest.raises(SystemExit):
        parse_args(["-n", "team1"])

def test_missing_name():
    with pytest.raises(SystemExit):
        parse_args(["-p", "4242"])

def test_reserved_name_graphic():
    with pytest.raises(SystemExit):
        parse_args(["-p", "4242", "-n", "GRAPHIC"])

def test_empty_name():
    with pytest.raises(SystemExit):
        parse_args(["-p", "4242", "-n", "   "])

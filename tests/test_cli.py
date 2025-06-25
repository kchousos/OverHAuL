# Copyright (C) 2025 Konstantinos Chousos
#
# This file is part of OverHAuL.
#
# OverHAuL is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# OverHAuL is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with OverHAuL.  If not, see <https://www.gnu.org/licenses/>.

import pytest
import os
from unittest import mock
from llm_harness.cli import parse_arguments
from llm_harness.config import Config


@pytest.fixture
def mock_argparse():
    """Fixture to mock argparse.ArgumentParser.parse_args"""
    with mock.patch("argparse.ArgumentParser.parse_args") as mock_parse:
        yield mock_parse


@pytest.fixture
def mock_os_path_exists():
    """Fixture to mock os.path.exists"""
    with mock.patch("os.path.exists") as mock_exists:
        mock_exists.return_value = True
        yield mock_exists


def test_parse_arguments_default_values(mock_argparse, mock_os_path_exists):
    """Test parse_arguments with default values."""
    # Setup mock return values
    mock_args = mock.MagicMock()
    mock_args.project = "test_project"
    mock_args.model = Config.DEFAULT_MODEL
    mock_args.files = Config.DEFAULT_FILES
    mock_argparse.return_value = mock_args

    # Call the function
    args = parse_arguments()

    # Assertions
    assert args.project_path == os.path.join(".", "assets", "test_project")
    assert args.model == Config.DEFAULT_MODEL
    assert args.file_patterns == Config.DEFAULT_FILES


def test_parse_arguments_custom_values(mock_argparse, mock_os_path_exists):
    """Test parse_arguments with custom values."""
    # Setup mock return values
    mock_args = mock.MagicMock()
    mock_args.project = "custom_project"
    mock_args.model = "gpt-4o"
    mock_args.files = ["*.cpp", "*.hpp"]
    mock_argparse.return_value = mock_args

    # Call the function
    args = parse_arguments()

    # Assertions
    assert args.project_path == os.path.join(".", "assets", "custom_project")
    assert args.model == "gpt-4o"
    assert args.file_patterns == ["*.cpp", "*.hpp"]


def test_parse_arguments_invalid_model(mock_argparse, mock_os_path_exists):
    """Test parse_arguments with invalid model."""
    # Setup mock return values
    mock_args = mock.MagicMock()
    mock_args.project = "test_project"
    mock_args.model = "invalid-model"
    mock_args.files = Config.DEFAULT_FILES
    mock_argparse.return_value = mock_args

    # Call the function
    args = parse_arguments()

    # Assertions - should fall back to default model
    assert args.model == Config.DEFAULT_MODEL


def test_parse_arguments_nonexistent_project(mock_argparse):
    """Test parse_arguments with nonexistent project."""
    # Setup mock return values
    mock_args = mock.MagicMock()
    mock_args.project = "nonexistent_project"
    mock_args.model = Config.DEFAULT_MODEL
    mock_args.files = Config.DEFAULT_FILES
    mock_argparse.return_value = mock_args

    # Mock os.path.exists to return False
    with mock.patch("os.path.exists", return_value=False):
        # Call the function - should raise FileNotFoundError
        with pytest.raises(FileNotFoundError):
            parse_arguments()

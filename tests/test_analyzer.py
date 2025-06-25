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
from unittest import mock
from llm_harness.core.analyzer import ProjectAnalyzer
from llm_harness.models.project import ProjectInfo
from llm_harness.config import Config


@pytest.fixture
def mock_glob():
    """Fixture to mock glob.glob"""
    with mock.patch("glob.glob") as mock_glob:
        yield mock_glob


@pytest.fixture
def mock_open():
    """Fixture to mock built-in open function"""
    mock_file = mock.mock_open(read_data="test file content")
    with mock.patch("builtins.open", mock_file):
        yield mock_file


class TestProjectAnalyzer:
    """Tests for the ProjectAnalyzer class."""

    def test_init_with_defaults(self):
        """Test initializing ProjectAnalyzer with default values."""
        analyzer = ProjectAnalyzer("/path/to/project")
        assert analyzer.project_path == "/path/to/project"
        assert analyzer.file_patterns == Config().DEFAULT_FILES

    def test_init_with_custom_patterns(self):
        """Test initializing ProjectAnalyzer with custom file patterns."""
        analyzer = ProjectAnalyzer("/path/to/project", ["*.cpp", "*.hpp"])
        assert analyzer.project_path == "/path/to/project"
        assert analyzer.file_patterns == ["*.cpp", "*.hpp"]

    def test_find_project_files(self, mock_glob):
        """Test _find_project_files method."""
        # Setup mock return values
        mock_glob.side_effect = [
            ["/path/to/project/file1.c", "/path/to/project/file2.c"],
            ["/path/to/project/header.h"],
        ]

        analyzer = ProjectAnalyzer(
            "/path/to/project", file_patterns=["*.c", "*.h"]
        )
        files = analyzer._find_project_files()

        # Assertions
        assert len(files) == 3
        assert "/path/to/project/file1.c" in files
        assert "/path/to/project/file2.c" in files
        assert "/path/to/project/header.h" in files
        assert mock_glob.call_count == 2

    def test_collect_project_info(self, mock_glob, mock_open):
        """Test collect_project_info method."""
        # Setup mock return values
        mock_glob.side_effect = [
            ["/path/to/project/file1.c"],
            ["/path/to/project/header.h"],
            ["/path/to/project/file2.cpp"],
            ["/path/to/project/header2.hpp"],
            [],
        ]

        analyzer = ProjectAnalyzer("/path/to/project")
        project_info = analyzer.collect_project_info()

        # Assertions
        assert isinstance(project_info, ProjectInfo)
        print(project_info.source)
        assert len(project_info.source) == 4

        # Check file names are extracted correctly
        assert project_info.source[0].name == "file1.c"
        assert project_info.source[1].name == "header.h"
        assert project_info.source[2].name == "file2.cpp"
        assert project_info.source[3].name == "header2.hpp"

        # Check content is read correctly
        assert project_info.source[0].content == "test file content"
        assert project_info.source[1].content == "test file content"
        assert project_info.source[2].content == "test file content"
        assert project_info.source[3].content == "test file content"

    def test_collect_project_info_empty(self, mock_glob):
        """Test collect_project_info with no matching files."""
        # Setup mock return values to return empty lists
        mock_glob.return_value = []

        analyzer = ProjectAnalyzer("/path/to/project", file_patterns=["*.c"])
        project_info = analyzer.collect_project_info()

        # Assertions
        assert isinstance(project_info, ProjectInfo)
        assert len(project_info.source) == 0

    def test_collect_project_info_read_error(self, mock_glob):
        """Test collect_project_info with file read error."""
        # Setup mock return values
        mock_glob.side_effect = [[]]

        # Mock open to raise an IOError
        with mock.patch("builtins.open", side_effect=IOError("Test IO Error")):
            analyzer = ProjectAnalyzer(
                "/path/to/project", file_patterns=["*.c"]
            )
            project_info = analyzer.collect_project_info()

            # Assertions - should return empty ProjectInfo
            assert isinstance(project_info, ProjectInfo)
            assert len(project_info.source) == 0

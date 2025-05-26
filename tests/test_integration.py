# Copyright (C) 2025 Konstantinos Chousos
#
# This file is part of LLM-Harness.
#
# LLM-Harness is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# LLM-Harness is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with LLM-Harness.  If not, see <https://www.gnu.org/licenses/>.

import pytest
import os
from unittest import mock
from llm_harness.core.analyzer import ProjectAnalyzer
from llm_harness.core.generator import HarnessGenerator
from llm_harness.io.file_manager import FileManager


@pytest.fixture
def mock_project_setup():
    """Fixture to create a mock project setup"""
    # Mock project path and files
    project_path = "/path/to/project"

    # Mock file content
    file_content = """
    #include "dateparse.h"
    
    int dateparse(const char* input) {
        // Implementation
        return 0;
    }
    """

    header_content = """
    #ifndef DATEPARSE_H
    #define DATEPARSE_H
    
    int dateparse(const char* input);
    
    #endif
    """

    # Mock find_project_files to return our test files
    with mock.patch.object(
        ProjectAnalyzer,
        "_find_project_files",
        return_value=[
            os.path.join(project_path, "dateparse.c"),
            os.path.join(project_path, "dateparse.h"),
        ],
    ):
        # Mock file open to return our test content
        mock_file = mock.mock_open()
        mock_file.side_effect = [
            mock.mock_open(read_data=file_content).return_value,
            mock.mock_open(read_data=header_content).return_value,
        ]

        with mock.patch("builtins.open", mock_file):
            yield project_path


@mock.patch("llm_harness.config.Config.load_env")
@mock.patch("dspy.LM")
@mock.patch("dspy.configure")
@mock.patch("os.makedirs")
def test_end_to_end_flow(
    mock_makedirs, mock_configure, mock_lm, mock_load_env, mock_project_setup
):
    """Test the end-to-end flow of the system."""
    project_path = mock_project_setup

    # Mock LLM response
    mock_lm_instance = mock.MagicMock()
    mock_lm_instance.return_value = ["Generated harness code"]
    mock_lm.return_value = mock_lm_instance

    # Mock API key
    mock_load_env.return_value = "test-api-key"

    # Mock file write
    with mock.patch("builtins.open", mock.mock_open()) as mock_file:
        # Create components and run flow
        analyzer = ProjectAnalyzer(project_path)
        project_info = analyzer.collect_project_info()

        generator = HarnessGenerator("gpt-4o")
        harness = generator.create_harness(project_info)

        file_manager = FileManager(project_path)
        result_path = file_manager.write_harness(harness)

        # Assertions
        assert len(project_info.source) == 2
        assert harness == "Generated harness code"
        assert result_path == os.path.join(
            project_path, "harnesses", "harness.c"
        )
        mock_file().write.assert_called_once_with("Generated harness code")
        mock_makedirs.assert_called_once_with(
            os.path.join(project_path, "harnesses"), exist_ok=True
        )

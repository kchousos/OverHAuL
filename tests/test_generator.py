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
from llm_harness.core.harnesser import Harnesser
from llm_harness.models.project import ProjectInfo, ProjectFile


class TestHarnessGenerator:
    """Tests for the HarnessGenerator class."""

    @mock.patch("llm_harness.config.Config.load_env")
    def test_init(self, mock_load_env):
        """Test initializing HarnessGenerator."""
        mock_load_env.return_value = "test-api-key"
        generator = Harnesser("gpt-4o")
        assert generator.model == "gpt-4o"
        mock_load_env.assert_called_once()

    @mock.patch("llm_harness.config.Config.load_env")
    @mock.patch("dspy.LM")
    @mock.patch("dspy.configure")
    def test_create_harness(self, mock_configure, mock_lm, mock_load_env):
        """Test create_harness method."""
        # Setup mocks
        mock_load_env.return_value = "test-api-key"
        mock_lm_instance = mock.MagicMock()
        mock_lm_instance.return_value = ["Generated harness code"]
        mock_lm.return_value = mock_lm_instance

        # Create test project info
        project_file = ProjectFile(
            path="/path/to/file.c",
            name="file.c",
            content="int main() { return 0; }",
        )
        project_info = ProjectInfo(source=[project_file])

        # Create generator and call method
        generator = Harnesser("gpt-4o")
        result = generator.harness(project_info)

        # Assertions
        assert result == "Generated harness code"
        mock_lm.assert_called_once_with("openai/gpt-4o", cache=False)
        mock_configure.assert_called_once()
        mock_lm_instance.assert_called_once()

    @mock.patch("llm_harness.config.Config.load_env")
    @mock.patch("dspy.LM")
    def test_create_harness_exception(self, mock_lm, mock_load_env):
        """Test create_harness method when an exception occurs."""
        # Setup mocks
        mock_load_env.return_value = "test-api-key"
        mock_lm.side_effect = Exception("Test exception")

        # Create test project info
        project_file = ProjectFile(
            path="/path/to/file.c",
            name="file.c",
            content="int main() { return 0; }",
        )
        project_info = ProjectInfo(source=[project_file])

        # Create generator and call method - should raise exception
        generator = Harnesser("gpt-4o")
        with pytest.raises(Exception, match="Test exception"):
            generator.harness(project_info)

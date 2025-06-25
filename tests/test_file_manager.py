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
from pathlib import Path
from llm_harness.io.file_manager import FileManager


class TestFileManager:
    """Tests for the FileManager class."""

    def test_init(self):
        """Test initializing FileManager."""
        manager = FileManager("/path/to/project")
        assert (
            Path(manager.project_path).resolve()
            == Path("/path/to/project").resolve()
        )
        assert (
            Path(manager.harness_dir).resolve()
            == Path("/path/to/project/harnesses").resolve()
        )

    @mock.patch("os.makedirs")
    @mock.patch("builtins.open", new_callable=mock.mock_open)
    def test_write_harness_default_filename(self, mock_file, mock_makedirs):
        """Test write_harness with default filename."""
        manager = FileManager("/path/to/project")
        result = manager.write_harness("harness content")

        # Assertions
        assert (
            Path(result).resolve()
            == Path("/path/to/project/harnesses/harness.c").resolve()
        )
        mock_makedirs.assert_called_once_with(
            "/path/to/project/harnesses", exist_ok=True
        )
        mock_file.assert_called_once_with(
            "/path/to/project/harnesses/harness.c", "w", encoding="utf-8"
        )
        mock_file().write.assert_called_once_with("harness content")

    @mock.patch("os.makedirs")
    @mock.patch("builtins.open", new_callable=mock.mock_open)
    def test_write_harness_custom_filename(self, mock_file, mock_makedirs):
        """Test write_harness with custom filename."""
        manager = FileManager("/path/to/project")
        result = manager.write_harness("harness content", "custom.c")

        # Assertions
        assert result == "/path/to/project/harnesses/custom.c"
        mock_makedirs.assert_called_once_with(
            os.path.join("/path/to/project", "harnesses"), exist_ok=True
        )
        mock_file.assert_called_once_with(
            "/path/to/project/harnesses/custom.c", "w", encoding="utf-8"
        )
        mock_file().write.assert_called_once_with("harness content")

    @mock.patch("os.makedirs")
    @mock.patch("builtins.open")
    def test_write_harness_io_error(self, mock_open, mock_makedirs):
        """Test write_harness when an IOError occurs."""
        mock_open.side_effect = IOError("Test IO Error")

        manager = FileManager("/path/to/project")

        # Call should raise the IOError
        with pytest.raises(IOError, match="Test IO Error"):
            manager.write_harness("harness content")

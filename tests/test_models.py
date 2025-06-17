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

from llm_harness.models.project import ProjectFile, ProjectInfo


class TestProjectModels:
    """Tests for the project models."""

    def test_project_file(self):
        """Test ProjectFile model."""
        file = ProjectFile(
            path="/path/to/file.c",
            name="file.c",
            content="int main() { return 0; }",
        )

        assert file.path == "/path/to/file.c"
        assert file.name == "file.c"
        assert file.content == "int main() { return 0; }"

    def test_project_info_empty(self):
        """Test ProjectInfo model with no files."""
        project_info = ProjectInfo(source=[])

        assert len(project_info.source) == 0
        assert project_info.get_source() == ""

    def test_project_info_single_file(self):
        """Test ProjectInfo model with a single file."""
        file = ProjectFile(
            path="/path/to/file.c",
            name="file.c",
            content="int main() { return 0; }",
        )
        project_info = ProjectInfo(source=[file])

        assert len(project_info.source) == 1
        concatenated = project_info.get_source()

        # Check that the content contains the expected header and file content
        assert "file.c" in concatenated
        assert "int main() { return 0; }" in concatenated

    def test_project_info_multiple_files(self):
        """Test ProjectInfo model with multiple files."""
        file1 = ProjectFile(
            path="/path/to/file1.c",
            name="file1.c",
            content="int func1() { return 1; }",
        )
        file2 = ProjectFile(
            path="/path/to/file2.h", name="file2.h", content="#define VALUE 42"
        )
        project_info = ProjectInfo(source=[file1, file2])

        assert len(project_info.source) == 2
        concatenated = project_info.get_source()

        # Check that the content contains both files with their headers
        assert "file1.c" in concatenated
        assert "int func1() { return 1; }" in concatenated
        assert "file2.h" in concatenated
        assert "#define VALUE 42" in concatenated

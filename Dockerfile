FROM python:3.10-slim

# Install system dependencies
RUN apt-get update && apt-get install -y \
    git \
    curl \
    build-essential \
    cppcheck \
    clang \
    clang-tools \
    libc++-dev \
    libc++abi-dev \
    llvm \
    && rm -rf /var/lib/apt/lists/*

# Install uv
COPY --from=ghcr.io/astral-sh/uv:latest /uv /bin/uv

# Set working directory
WORKDIR /app

# Copy dependency files first (for better layer caching)
COPY pyproject.toml uv.lock README.md ./

# Install dependencies first
RUN uv sync --frozen --no-group dev --no-group docs

# Copy source code
COPY src/ ./src/

# Install the package
RUN uv pip install .

# Set environment variables
ENV UV_SYSTEM_PYTHON=1
ENV PATH="/app/.venv/bin:$PATH"
ENV FORCE_COLOR=1
ENV TERM=xterm-256color

# Use the CLI command
ENTRYPOINT ["overhaul"]
CMD ["--help"]

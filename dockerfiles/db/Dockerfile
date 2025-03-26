FROM debian:stable

# Install necessary dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    curl \
    file \
    git \
    wget \
    libreadline-dev \
    zlib1g-dev \
    flex \
    bison \
    libxml2-dev \
    libxslt-dev \
    libssl-dev \
    libpq-dev \
    clang \
    llvm \
    gdb \
    cmake \
    python3 \
    python3-pip \
    ccache \
    sudo \
    && rm -rf /var/lib/apt/lists/*

# Set environment variables
ENV POSTGRES_REPO="https://github.com/postgres/postgres.git"
# ENV COMMIT_HASH="c366d2bdba7c3b9b2cca1429d4535866e231ca55"
ENV INSTALL_PREFIX="/usr/local/pgsql"
ENV PGDATA="/var/lib/postgresql/data"

# Create a non-root user (just like the official image)
RUN useradd -m -d /var/lib/postgresql -s /bin/bash postgres && \
    mkdir -p $PGDATA && \
    chown -R postgres:postgres /var/lib/postgresql

# Clone PostgreSQL source code and checkout specific commit
WORKDIR /usr/src
RUN git clone --depth 1 ${POSTGRES_REPO} postgres

# Build PostgreSQL with debugging symbols
WORKDIR /usr/src/postgres
RUN ./configure --prefix=${INSTALL_PREFIX} --enable-debug --enable-cassert --without-icu && \
    make -j$(nproc) && \
    make install && \
    cd contrib/pgcrypto && \
    make && make install

# Add PostgreSQL binaries to the path
ENV PATH="${INSTALL_PREFIX}/bin:${PATH}"
ENV LD_LIBRARY_PATH="${INSTALL_PREFIX}/lib"

WORKDIR /home/app

ENV HOME=/home/app \
  PATH=/home/app/.cargo/bin:/home/linuxbrew/.linuxbrew/bin:$PATH
    
RUN chown postgres:postgres /home/app
RUN usermod -aG sudo postgres
RUN echo "postgres ALL=(ALL) NOPASSWD:ALL" >> /etc/sudoers
    
USER postgres

RUN git clone https://github.com/automerge/automerge
RUN cd automerge

RUN curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh -s -- -y
RUN /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

RUN test -d /home/linuxbrew/.linuxbrew && eval "$(/home/linuxbrew/.linuxbrew/bin/brew shellenv)"
RUN echo "eval \"\$($(brew --prefix)/bin/brew shellenv)\"" >> ~/.bashrc

RUN brew install node cmocka

RUN npm install --global yarn

RUN mkdir javascript
RUN yarn --verbose --cwd ./javascript

RUN cargo install wasm-bindgen-cli wasm-opt cargo-deny

RUN rustup toolchain install nightly
RUN rustup component add rust-src --toolchain nightly

RUN rustup target add wasm32-unknown-unknown

RUN cd /home/app/automerge/rust
RUN cmake -E make_directory /home/app/automerge/rust/automerge-c/build
RUN cmake -S /home/app/automerge/rust/automerge-c -B /home/app/automerge/rust/automerge-c/build -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=ON
RUN cmake --build /home/app/automerge/rust/automerge-c/build 
RUN sudo cmake --install /home/app/automerge/rust/automerge-c/build --prefix "/usr"

RUN cd ../..
RUN ls
COPY . .
RUN sudo chown -R postgres:postgres src
RUN make && sudo make install PG_CONFIG=/usr/local/pgsql/bin/pg_config

# Expose PostgreSQL port
EXPOSE 5432

# Default entrypoint script
RUN sudo cp dockerfiles/db/entrypoint.sh /usr/local/pgsql/bin/
RUN sudo chmod +x /usr/local/pgsql/bin/entrypoint.sh

ENTRYPOINT ["/usr/local/pgsql/bin/entrypoint.sh"]
CMD ["postgres"]

# Use the official Ubuntu image as a base
FROM ubuntu:latest

# Set the working directory inside the container
WORKDIR /app

# Install necessary packages for building and running RPC
RUN apt-get update && apt-get install -y \
    build-essential \
    rpcbind \
    libtirpc-dev \
    gcc \
    make \
    net-tools \
    sudo \
    && rm -rf /var/lib/apt/lists/*

# Create necessary directories for rpcbind and set correct ownership
RUN mkdir -p /run/rpcbind /run/sendsigs.omit.d && \
    chown root:root /run/rpcbind /run/sendsigs.omit.d

# Expose necessary ports
EXPOSE 50051

# Copy the project files into the container
COPY . /app

# Verify that the Makefile and other project files are copied correctly
RUN ls -l /app

# Run the Makefile to build the project
RUN make -f Makefile.add

# Set the ownership of /run/rpcbind to root:root
RUN chown -R root:root /run/rpcbind /run/sendsigs.omit.d

# Start rpcbind and the server
CMD ["sh", "-c", "chown root:root /run/rpcbind && rpcbind && ./add_server 0.0.0.0:50051"]

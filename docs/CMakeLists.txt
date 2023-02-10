add_custom_target(
        docs
        COMMAND sh -c "mandoc -T html -O style=mandoc.css palera1n.1 > palera1n.1.html"
)

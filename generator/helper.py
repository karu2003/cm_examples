import struct


def pack_data(args, g_start):
    send_data = struct.pack(
        "<iiiiiffffffi??i",
        args.sample_rate_hz,
        args.sample_format.id,
        args.dma_buffer_size_ms,
        args.num_dma_buffers,
        args.drop_first_samples_ms,
        args.dac_sample_rate_hz,
        args.duration,
        args.fstart,
        args.fstop,
        args.amp,
        args.phi,
        args.signal_type.value,
        args.auto_restart,
        args.run_back,
        g_start,
    )
    return send_data

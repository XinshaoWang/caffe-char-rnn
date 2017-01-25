#include <gflags/gflags.h>
#include "Classifier.h"
#include "Trainer.h"

//General flags
DEFINE_bool(train, true, "True to train, false to generate new text");
DEFINE_bool(gpu, true, "Use GPU to brew Caffe");
DEFINE_int32(sequence_length, 75, "Number of characters to consider when predicting");
DEFINE_int32(batch_size, 25, "Number of sequences in one batch");

//Train flags
DEFINE_string(solver, "", "Solver file to use to train (*.prototxt)");
DEFINE_string(snapshot, "", "Snapshot file to resume training (*.snapshot)");
DEFINE_string(logfile, "", "File to log the loss every log_interval iterations");
DEFINE_int32(log_interval, 100, "Every log_interval iterations, log the loss in logfile");
DEFINE_string(textfile, "", "Raw text file for training");

//Test flags
DEFINE_double(temperature, 0.0, "Temperature for character selection (0.0..1.0), 0.0 makes the classifier always select the character with the highest probability");
DEFINE_string(vocabulary, "vocabulary.txt", "The vocabulary of all the possible characters");
DEFINE_string(model, "", "Net model file (*.prototxt)");
DEFINE_string(weights, "", "Weights of the model to load");
DEFINE_string(seed, "", "Initial string to launch the model (random if none");
DEFINE_int32(number_of_predictions, 100, "Number of characters generated by the model");
DEFINE_string(output_file, "", "Predicted characters are written in this file if not empty");

int main(int argc, char** argv)
{
	gflags::ParseCommandLineFlags(&argc, &argv, true);
	google::InitGoogleLogging(argv[0]);

	//In test mode we don't need all the caffe stuff
	if (!FLAGS_train)
	{
		for (int i = 0; i < google::NUM_SEVERITIES; ++i)
		{
			google::SetLogDestination(i, "");
		}
	}
	else
	{
		google::LogToStderr();
	}

	if (FLAGS_gpu)
	{
		caffe::Caffe::set_mode(caffe::Caffe::GPU);
	}
	else
	{
		caffe::Caffe::set_mode(caffe::Caffe::CPU);
	}


	if (FLAGS_train)
	{
		Trainer trainer(FLAGS_solver, FLAGS_snapshot, FLAGS_textfile, FLAGS_logfile, FLAGS_log_interval, FLAGS_sequence_length, FLAGS_batch_size);
		int iteration = 0;

		caffe::SolverParameter solver_param;
		caffe::ReadProtoFromTextFileOrDie(FLAGS_solver, &solver_param);

		while (iteration < solver_param.max_iter())
		{
			trainer.Update();
			iteration++;
		}
	}
	else
	{
		Classifier classifier(FLAGS_model, FLAGS_weights, FLAGS_vocabulary, FLAGS_sequence_length, FLAGS_batch_size, FLAGS_temperature, FLAGS_output_file);

		std::vector<char> inputChar(FLAGS_seed.begin(), FLAGS_seed.end());

		std::cout << std::endl << std::endl << FLAGS_seed;

		std::vector<char> output = classifier.Predict(inputChar, FLAGS_number_of_predictions, true);
	}

	return 0;

}
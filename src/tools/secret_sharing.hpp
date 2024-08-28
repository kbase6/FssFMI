/**
 * @file secret_sharing.hpp
 * @author tomo-uchiyama@moegi.waseda.jp
 * @date 2024-01-05
 * @copyright Copyright (c) 2024
 * @brief Secret Sharing class.
 */

#ifndef SECRET_SHARING_H_
#define SECRET_SHARING_H_

#include <array>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

#include "../comm/client.hpp"
#include "../comm/comm.hpp"
#include "../comm/server.hpp"
#include "../utils/file_io.hpp"

namespace tools {
namespace secret_sharing {

using share_t  = std::pair<uint32_t, uint32_t>;
using shares_t = std::pair<std::vector<uint32_t>, std::vector<uint32_t>>;

class Party {
public:
    /**
     * @brief Constructs a Party object using communication information.
     *
     * Initializes a Party object based on communication information containing the party's ID, server, and client details.
     *
     * @param comm_info A reference to a CommInfo object containing communication details like party ID, port number, and host address.
     */
    Party(const comm::CommInfo &comm_info);

    /**
     * @brief Initiates communication setup for the Party object.
     *
     * This method initializes the communication setup for the Party object based on its ID.
     * It starts the communication between server and client.
     *
     */
    void StartCommunication(const bool debug = false);

    /**
     * @brief Ends communication for the Party object.
     *
     * This method ends the communication for the Party object by closing the server and client connections.
     */
    void EndCommunication();

    /**
     * @brief Retrieves the ID of the Party object.
     *
     * This method returns the ID associated with the Party object.
     *
     * @return An integer representing the ID of the Party object.
     */
    uint32_t GetId() const;

    /**
     * @brief Sends and receives data between the two parties.
     *
     * This method facilitates the exchange of data between the two parties in the communication protocol.
     *
     * @param x_0 A reference to an unsigned 32-bit integer representing the value to be sent/received.
     * @param x_1 A reference to an unsigned 32-bit integer where the received value will be stored.
     */
    void SendRecv(uint32_t &x_0, uint32_t &x_1);

    /**
     * @brief Sends and receives vectors of data between the two parties.
     *
     * This method facilitates the exchange of vectors of unsigned 32-bit integers between the two parties
     * in the communication protocol.
     *
     * @param x_vec_0 A reference to a vector of unsigned 32-bit integers to be sent/received.
     * @param x_vec_1 A reference to a vector of unsigned 32-bit integers where the received values will be stored.
     */
    void SendRecv(std::vector<uint32_t> &x_vec_0, std::vector<uint32_t> &x_vec_1);

    /**
     * @brief Sends and receives arrays of data between the two parties.
     *
     * This method facilitates the exchange of arrays of unsigned 32-bit integers between the two parties
     * in the communication protocol.
     *
     * @param x_arr_0 A reference to a array of unsigned 32-bit integers to be sent/received.
     * @param x_arr_1 A reference to a array of unsigned 32-bit integers where the received values will be stored.
     */
    void SendRecv(std::array<uint32_t, 2> &x_arr_0, std::array<uint32_t, 2> &x_arr_1);

    /**
     * @brief Sends and receives arrays of data between the two parties.
     *
     * This method facilitates the exchange of arrays of unsigned 32-bit integers between the two parties
     * in the communication protocol.
     *
     * @param x_arr_0 A reference to a array of unsigned 32-bit integers to be sent/received.
     * @param x_arr_1 A reference to a array of unsigned 32-bit integers where the received values will be stored.
     */
    void SendRecv(std::array<uint32_t, 4> &x_arr_0, std::array<uint32_t, 4> &x_arr_1);

    uint32_t GetTotalBytesSent() const;

    uint32_t OutputTotalBytesSent(const std::string &message) const;

    /**
     * @brief Clears the total number of bytes sent by the party.
     *
     * Resets the total number of bytes sent by the party to zero.
     */
    void ClearTotalBytesSent();

private:
    const uint32_t id_;         /**< ID of the party. */
    comm::Server   p0_;         /**< Server communication instance. */
    comm::Client   p1_;         /**< Client communication instance. */
    bool           is_started_; /**< Flag indicating whether the communication has started. */
};

struct BeaverTriplet {
    uint32_t a;
    uint32_t b;
    uint32_t c;

    /**
     * @brief Constructs a BeaverTriplet object initializing values a, b, and c to 0.
     *
     * This constructor initializes a BeaverTriplet object with three unsigned 64-bit integers: a, b, and c.
     * It sets the values of a, b, and c to 0.
     */
    BeaverTriplet();

    /**
     * @brief Constructs a BeaverTriplet object initializing values a, b, and c with specified parameters.
     *
     * This constructor initializes a BeaverTriplet object with three unsigned 32-bit integers: val_a, val_b, and val_c.
     * It assigns the values of val_a, val_b, and val_c to the corresponding variables a, b, and c.
     *
     * @param val_a The value for the 'a' component of the BeaverTriplet.
     * @param val_b The value for the 'b' component of the BeaverTriplet.
     * @param val_c The value for the 'c' component of the BeaverTriplet.
     */
    BeaverTriplet(uint32_t val_a, uint32_t val_b, uint32_t val_c);

    /**
     * @brief Generates a string representation of the BeaverTriplet object.
     *
     * This method generates a string representation of the BeaverTriplet object based on the boolean parameter 'sup'.
     * If 'sup' is true, it returns a supplementary format string "(a, b, c) = (val_a, val_b, val_c)".
     * If 'sup' is false, it returns a standard string representation "(val_a, val_b, val_c)".
     *
     * @param sup Indicates whether to generate a supplementary format string (true) or a standard string representation (false).
     * @return A string representation of the BeaverTriplet object.
     */
    std::string ToStr(const bool sup = true) const;
};

using bts_t = std::vector<BeaverTriplet>;

class AdditiveSecretSharing {

public:
    /**
     * @brief Default constructor for AdditiveSecretSharing class.
     *
     * Initializes a AdditiveSecretSharing object with a default bit size of 32.
     */
    AdditiveSecretSharing();

    /**
     * @brief Constructor for AdditiveSecretSharing class with custom bit size.
     *
     * Initializes a AdditiveSecretSharing object with the specified bit size.
     *
     * @param bitsize The size of the bits used for secret sharing operations.
     */
    AdditiveSecretSharing(uint32_t bitsize);

    /**
     * @brief Shares a secret value using secret sharing.
     *
     * Splits the given secret value 'x' into two shares, 'x_0' and 'x_1',
     * using secret sharing techniques based on the bit size of the secret sharing object.
     *
     * @param x The secret value to be shared.
     * @return A pair of shares representing the secret value.
     */
    share_t Share(const uint32_t x) const;

    /**
     * @brief Reconstructs a secret value from its shares.
     *
     * Reconstructs the secret value from its shares 'x_0' and 'x_1' using secret sharing techniques.
     *
     * @param party The Party object representing the party that will perform the reconstruction.
     * @param x_0 The first share of the secret value.
     * @param x_1 The second share of the secret value.
     * @return The reconstructed secret value.
     */
    uint32_t Reconst(Party &party, uint32_t x_0, uint32_t x_1) const;

    /**
     * @brief Shares a vector of secret values using secret sharing.
     *
     * Splits the given vector of secret values 'x_vec' into two share vectors, 'x_vec_0' and 'x_vec_1',
     * using secret sharing techniques based on the bit size of the secret sharing object.
     *
     * @param x_vec The vector of secret values to be shared.
     * @return A pair of share vectors representing the secret values.
     */
    shares_t Share(const std::vector<uint32_t> &x_vec) const;

    /**
     * @brief Reconstructs a vector of secret values from their shares.
     *
     * Reconstructs the vector of secret values from their share vectors 'x_vec_0' and 'x_vec_1' using secret sharing techniques.
     *
     * @param party The Party object representing the party that will perform the reconstruction.
     * @param x_vec_0 The first share vector of the secret values.
     * @param x_vec_1 The second share vector of the secret values.
     * @param output The reconstructed vector of secret values.
     */
    void Reconst(Party &party, std::vector<uint32_t> &x_vec_0, std::vector<uint32_t> &x_vec_1, std::vector<uint32_t> &output) const;

    /**
     * @brief Shares an array of secret values using secret sharing.
     *
     * Splits the given array of secret values 'x_arr' into two share arrays, 'x_arr_0' and 'x_arr_1',
     * using secret sharing techniques based on the bit size of the secret sharing object.
     *
     * @param x_arr The array of secret values to be shared.
     * @return A pair of share arrays representing the secret values.
     */
    void Reconst(Party &party, std::array<uint32_t, 2> &x_arr_0, std::array<uint32_t, 2> &x_arr_1, std::array<uint32_t, 2> &output) const;

    /**
     * @brief Reconstructs an array of secret values from their shares.
     *
     * Reconstructs the array of secret values from their share arrays 'x_arr_0' and 'x_arr_1' using secret sharing techniques.
     *
     * @param party The Party object representing the party that will perform the reconstruction.
     * @param x_arr_0 The first share array of the secret values.
     * @param x_arr_1 The second share array of the secret values.
     * @param output The reconstructed array of secret values.
     */
    void Reconst(Party &party, std::array<uint32_t, 4> &x_arr_0, std::array<uint32_t, 4> &x_arr_1, std::array<uint32_t, 4> &output) const;

    /**
     * @brief Generates Beaver triples.
     *
     * Generates a vector of 'bt_num' Beaver triples using the secret sharing techniques.
     *
     * @param bt_num The number of Beaver triples to generate.
     * @param bt_vec The vector to store the generated Beaver triples.
     */
    void GenerateBeaverTriples(const uint32_t bt_num, bts_t &bt_vec) const;

    /**
     * @brief Shares Beaver triples using secret sharing.
     *
     * Splits the given vector of Beaver triples 'bt_vec' into two share vectors, 'bt_vec_0' and 'bt_vec_1',
     * using secret sharing techniques based on the bit size of the secret sharing object.
     *
     * @param bt_vec The vector of Beaver triples to be shared.
     * @return A pair of share vectors representing the Beaver triples.
     */
    std::pair<bts_t, bts_t> ShareBeaverTriples(const bts_t &bt_vec) const;

    /**
     * @brief Performs secure multiplication of two secret-shared values.
     *
     * Performs secure multiplication of two secret-shared values 'x' and 'y' using a Beaver triplet 'bt'.
     * The result is also secret-shared and returned.
     *
     * @param party The party object representing the current party.
     * @param bt The Beaver triplet used for secure multiplication.
     * @param x The secret-shared value of the first operand.
     * @param y The secret-shared value of the second operand.
     * @return The secret-shared result of the multiplication.
     */
    uint32_t Mult(Party &party, const BeaverTriplet &bt, const uint32_t x, const uint32_t y) const;

    /**
     * @brief Performs secure multiplication of two pairs of secret-shared values.
     *
     * Performs secure multiplication of two pairs of secret-shared values '(x1, y1)' and '(x2, y2)'
     * using two Beaver triplets 'bt1' and 'bt2'. The result is also secret-shared and returned as a pair.
     *
     * @param party The party object representing the current party.
     * @param bt1 The first Beaver triplet used for secure multiplication.
     * @param bt2 The second Beaver triplet used for secure multiplication.
     * @param x1 The secret-shared value of the first operand (x-coordinate).
     * @param y1 The secret-shared value of the first operand (y-coordinate).
     * @param x2 The secret-shared value of the second operand (x-coordinate).
     * @param y2 The secret-shared value of the second operand (y-coordinate).
     * @return An array representing the secret-shared result of the multiplication as [z1, z2].
     */
    std::array<uint32_t, 2> Mult2(Party &party, const BeaverTriplet &bt1, const BeaverTriplet &bt2, const uint32_t x1, const uint32_t y1, const uint32_t x2, const uint32_t y2) const;

    /**
     * @brief Performs secure multiplication of two vectors of secret-shared values.
     *
     * Performs secure multiplication of two vectors of secret-shared values 'x' and 'y' using a vector of Beaver triplets 'bt_vec'.
     * The result is also secret-shared and returned as a vector.
     *
     * @param party The party object representing the current party.
     * @param bt_vec The vector of Beaver triplets used for secure multiplication.
     * @param x The vector of secret-shared values of the first operand.
     * @param y The vector of secret-shared values of the second operand.
     * @param z The vector to store the secret-shared result of the multiplication.
     */
    void Mult(Party &party, const bts_t &bt_vec, const std::vector<uint32_t> &x, const std::vector<uint32_t> &y, std::vector<uint32_t> &z) const;

private:
    uint32_t bitsize_;
};

class BooleanSecretSharing {
public:
    /**
     * @brief Default constructor for BooleanSecretSharing class.
     *
     * Initializes a BooleanSecretSharing object with a default bit size of 32.
     */
    BooleanSecretSharing(){};

    /**
     * @brief Shares a secret value using secret sharing.
     *
     * Splits the given secret value 'x' into two shares, 'x_0' and 'x_1',
     * using secret sharing techniques based on the bit size of the secret sharing object.
     *
     * @param x The secret value to be shared.
     * @return A pair of shares representing the secret value.
     */
    share_t Share(const uint32_t x) const;

    /**
     * @brief Reconstructs a secret value from its shares.
     *
     * Reconstructs the secret value from its shares 'x_0' and 'x_1' using secret sharing techniques.
     *
     * @param party The Party object representing the party that will perform the reconstruction.
     * @param x_0 The first share of the secret value.
     * @param x_1 The second share of the secret value.
     * @return The reconstructed secret value.
     */
    uint32_t Reconst(Party &party, uint32_t x_0, uint32_t x_1) const;

    /**
     * @brief Shares a vector of secret values using secret sharing.
     *
     * Splits the given vector of secret values 'x_vec' into two share vectors, 'x_vec_0' and 'x_vec_1',
     * using secret sharing techniques based on the bit size of the secret sharing object.
     *
     * @param x_vec The vector of secret values to be shared.
     * @return A pair of share vectors representing the secret values.
     */
    shares_t Share(const std::vector<uint32_t> &x_vec) const;

    /**
     * @brief Reconstructs a vector of secret values from their shares.
     *
     * Reconstructs the vector of secret values from their share vectors 'x_vec_0' and 'x_vec_1' using secret sharing techniques.
     *
     * @param party The Party object representing the party that will perform the reconstruction.
     * @param x_vec_0 The first share vector of the secret values.
     * @param x_vec_1 The second share vector of the secret values.
     * @param output The reconstructed vector of secret values.
     */
    void Reconst(Party &party, std::vector<uint32_t> &x_vec_0, std::vector<uint32_t> &x_vec_1, std::vector<uint32_t> &output) const;

    /**
     * @brief Shares an array of secret values using secret sharing.
     *
     * Splits the given array of secret values 'x_arr' into two share arrays, 'x_arr_0' and 'x_arr_1',
     * using secret sharing techniques based on the bit size of the secret sharing object.
     *
     * @param x_arr The array of secret values to be shared.
     * @return A pair of share arrays representing the secret values.
     */
    void Reconst(Party &party, std::array<uint32_t, 2> &x_arr_0, std::array<uint32_t, 2> &x_arr_1, std::array<uint32_t, 2> &output) const;

    /**
     * @brief Reconstructs an array of secret values from their shares.
     *
     * Reconstructs the array of secret values from their share arrays 'x_arr_0' and 'x_arr_1' using secret sharing techniques.
     *
     * @param party The Party object representing the party that will perform the reconstruction.
     * @param x_arr_0 The first share array of the secret values.
     * @param x_arr_1 The second share array of the secret values.
     * @param output The reconstructed array of secret values.
     */
    void Reconst(Party &party, std::array<uint32_t, 4> &x_arr_0, std::array<uint32_t, 4> &x_arr_1, std::array<uint32_t, 4> &output) const;

    /**
     * @brief Generates Beaver triples.
     *
     * Generates a vector of 'bt_num' Beaver triples using the secret sharing techniques.
     *
     * @param bt_num The number of Beaver triples to generate.
     * @param bt_vec The vector to store the generated Beaver triples.
     */
    void GenerateBeaverTriples(const uint32_t bt_num, bts_t &bt_vec) const;

    /**
     * @brief Shares Beaver triples using secret sharing.
     *
     * Splits the given vector of Beaver triples 'bt_vec' into two share vectors, 'bt_vec_0' and 'bt_vec_1',
     * using secret sharing techniques based on the bit size of the secret sharing object.
     *
     * @param bt_vec The vector of Beaver triples to be shared.
     * @return A pair of share vectors representing the Beaver triples.
     */
    std::pair<bts_t, bts_t> ShareBeaverTriples(const bts_t &bt_vec) const;

    /**
     * @brief Performs secure bitwise AND operation on two secret-shared boolean values.
     *
     * Performs secure bitwise AND operation on two secret-shared boolean values 'x_b' and 'y_b'
     * using a Beaver triplet 'bt_b'. The result is also secret-shared and returned.
     *
     * @param party The party object representing the current party.
     * @param bt_b The Beaver triplet used for secure bitwise AND operation.
     * @param x_b The secret-shared boolean value of the first operand.
     * @param y_b The secret-shared boolean value of the second operand.
     * @return The secret-shared result of the bitwise AND operation.
     */
    uint32_t And(Party &party, const BeaverTriplet &bt_b, const uint32_t x_b, const uint32_t y_b) const;

    /**
     * @brief Performs secure bitwise AND operation on multiple pairs of secret-shared boolean values.
     *
     * Performs secure bitwise AND operation on multiple pairs of secret-shared boolean values represented
     * by vectors 'xb_vec' and 'yb_vec', using corresponding Beaver triplets in 'btb_vec'. The results are
     * also secret-shared and stored in the vector 'zb_vec'.
     *
     * @param party The party object representing the current party.
     * @param btb_vec The vector of Beaver triplets used for secure bitwise AND operations.
     * @param xb_vec The vector of secret-shared boolean values of the first operands.
     * @param yb_vec The vector of secret-shared boolean values of the second operands.
     * @param zb_vec The vector to store the secret-shared results of the bitwise AND operations.
     */
    void And(Party &party, const bts_t &btb_vec, const std::vector<uint32_t> &xb_vec, const std::vector<uint32_t> &yb_vec, std::vector<uint32_t> &zb_vec) const;

    /**
     * @brief Performs secure bitwise OR operation on two secret-shared boolean values.
     *
     * Performs secure bitwise OR operation on two secret-shared boolean values 'x_b' and 'y_b'
     * using a Beaver triplet 'bt_b'. The result is also secret-shared and returned.
     *
     * @param party The party object representing the current party.
     * @param bt_b The Beaver triplet used for secure bitwise OR operation.
     * @param x_b The secret-shared boolean value of the first operand.
     * @param y_b The secret-shared boolean value of the second operand.
     * @return The secret-shared result of the bitwise OR operation.
     */
    uint32_t Or(Party &party, const BeaverTriplet &bt_b, const uint32_t x_b, const uint32_t y_b) const;

    /**
     * @brief Performs secure bitwise OR operation on multiple pairs of secret-shared boolean values.
     *
     * Performs secure bitwise OR operation on multiple pairs of secret-shared boolean values represented
     * by vectors 'xb_vec' and 'yb_vec', using corresponding Beaver triplets in 'btb_vec'. The results are
     * also secret-shared and stored in the vector 'zb_vec'.
     *
     * @param party The party object representing the current party.
     * @param btb_vec The vector of Beaver triplets used for secure bitwise OR operations.
     * @param xb_vec The vector of secret-shared boolean values of the first operands.
     * @param yb_vec The vector of secret-shared boolean values of the second operands.
     * @param zb_vec The vector to store the secret-shared results of the bitwise OR operations.
     */
    void Or(Party &party, const bts_t &btb_vec, const std::vector<uint32_t> &xb_vec, const std::vector<uint32_t> &yb_vec, std::vector<uint32_t> &zb_vec) const;
};

class ShareHandler {
public:
    /**
     * @brief Construct a new Share Handler object
     *
     * @param debug Flag indicating whether to print debug messages.
     * @param io_debug Flag indicating whether to print debug messages for file I/O operations.
     * @param ext File extension to use for file I/O operations.
     */
    ShareHandler(const bool debug = false, const bool io_debug = false, const std::string ext = ".dat");

    /**
     * @brief Exports share values to files.
     *
     * Exports the share values stored in the pair 'x_sh' to two separate files specified by 'file_path_p0' and 'file_path_p1'.
     *
     * @param file_path_p0 The file path for the first share value.
     * @param file_path_p1 The file path for the second share value.
     * @param x_sh The pair containing the share values to be exported.
     */
    void ExportShare(const std::string &file_path_p0, const std::string &file_path_p1, share_t &x_sh);

    /**
     * @brief Exports share vectors to files.
     *
     * Exports the share vectors stored in the pair 'x_vec_sh' to two separate files specified by 'file_path_p0' and 'file_path_p1'.
     *
     * @param file_path_p0 The file path for the first share vector.
     * @param file_path_p1 The file path for the second share vector.
     * @param x_vec_sh The pair containing the share vectors to be exported.
     */
    void ExportShare(const std::string &file_path_p0, const std::string &file_path_p1, shares_t &x_vec_sh);

    /**
     * @brief Loads a share value from a file.
     *
     * Loads a share value from the file specified by 'file_path' and stores it in 'x_sh'.
     *
     * @param file_path The file path from which to load the share value.
     * @param x_sh Reference to the variable to store the loaded share value.
     */
    void LoadShare(const std::string &file_path, uint32_t &x_sh);

    /**
     * @brief Loads a share vector from a file.
     *
     * Loads a share vector from the file specified by 'file_path' and stores it in 'x_vec_sh'.
     *
     * @param file_path The file path from which to load the share vector.
     * @param x_vec_sh Reference to the vector to store the loaded share vector.
     */
    void LoadShare(const std::string &file_path, std::vector<uint32_t> &x_vec_sh);

    /**
     * @brief Exports Beaver triples to a file.
     *
     * Exports the Beaver triples stored in 'bt_vec' to the file specified by 'file_path'.
     *
     * @param file_path The file path to export the Beaver triples.
     * @param bt_vec Reference to the vector containing the Beaver triples.
     */
    void ExportBT(const std::string &file_path, bts_t &bt_vec);

    /**
     * @brief Exports Beaver triple shares to files.
     *
     * Exports the Beaver triple shares stored in 'bt_vec_sh' to two separate files specified by 'file_path_p0' and 'file_path_p1'.
     *
     * @param file_path_p0 The file path for the first Beaver triple share vector.
     * @param file_path_p1 The file path for the second Beaver triple share vector.
     * @param bt_vec_sh The pair containing the Beaver triple share vectors to be exported.
     */
    void ExportBTShare(const std::string &file_path_p0, const std::string &file_path_p1, std::pair<bts_t, bts_t> &bt_vec_sh);

    /**
     * @brief Loads Beaver triple shares from a file.
     *
     * Loads Beaver triple shares from the file specified by 'file_path' and stores them in 'bt_vec_sh'.
     *
     * @param file_path The file path from which to load the Beaver triple shares.
     * @param bt_vec_sh Reference to the vector to store the loaded Beaver triple shares.
     */
    void LoadBTShare(const std::string &file_path, bts_t &bt_vec_sh);

private:
    const bool    debug_; /**< Flag indicating whether to print debug messages. */
    utils::FileIo io_;    /**< File I/O utility object. */

    /**
     * @brief Writes Beaver triples to a file.
     *
     * Writes the Beaver triples stored in 'bt_vec' to the file specified by 'file_path'.
     *
     * @param file_path The file path to write the Beaver triples.
     * @param bt_vec Reference to the vector containing the Beaver triples.
     */
    void WriteBeaverTriplesToFile(const std::string &file_path, bts_t &bt_vec);
    /**
     * @brief Reads Beaver triples from a file.
     *
     * Reads the Beaver triples from the file specified by 'file_path' and stores them in 'bt_vec'.
     *
     * @param file_path The file path from which to read the Beaver triples.
     * @param bt_vec Reference to the vector to store the read Beaver triples.
     */
    void ReadBeaverTriplesFromFile(const std::string &file_path, bts_t &bt_vec);
};

}    // namespace secret_sharing
}    // namespace tools

#endif    // SECRET_SHARING_H_
